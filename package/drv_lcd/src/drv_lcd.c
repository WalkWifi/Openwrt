#include <linux/module.h>  
#include <linux/types.h>  
#include <linux/fs.h>  
#include <linux/errno.h>  
#include <linux/mm.h>  
#include <linux/sched.h>  
#include <linux/init.h>  
#include <linux/cdev.h>  
#include <asm/io.h>  
#include <asm/uaccess.h>  
#include <linux/timer.h>  
#include <asm/atomic.h>  
#include <linux/slab.h>  
#include <linux/device.h>  
#include <linux/delay.h>

#include "drv_lcd.h"

//V1.1	//2015.09.16
//V2.0	//2015.12.21
#define LCD_VERSION "2.0"	//2015.09.16

#define EOS_OK  0
#define EOS_FAIL 1

#define THE_DEBUG 0
#define DEV_NAME    "lcd"
#define SLAVE_ADDR    0x7A	//A0=1,add=0x7A;A0=0,add=0x78;

#define GPIO_SDA_PIN	5
#define GPIO_SCL_PIN	4
#define GPIO_RST_PIN	6

#define GPIO1_MODE_ADDR 0xB0000060
#define GPIO0_CTRL_ADD 0XB0000600
#define GPIO0_DATA_ADD 0XB0000620	//read or write data

#define IIC_RST_OUT() (*(unsigned int *)GPIO0_CTRL_ADD |= (1 << GPIO_RST_PIN))
#define IIC_SDA_OUT() (*(unsigned int *)GPIO0_CTRL_ADD |= (1 << GPIO_SDA_PIN))
#define IIC_SDA_IN()  (*(unsigned int *)GPIO0_CTRL_ADD &= ~(1 << GPIO_SDA_PIN))
#define IIC_SCL_OUT() (*(unsigned int *)GPIO0_CTRL_ADD |= (1 << GPIO_SCL_PIN))

#define IIC_RST_SET() (*(unsigned int *)GPIO0_DATA_ADD |= (1 << GPIO_RST_PIN))
#define IIC_RST_CLR() (*(unsigned int *)GPIO0_DATA_ADD &= ~(1 << GPIO_RST_PIN))
#define IIC_SDA_SET() (*(unsigned int *)GPIO0_DATA_ADD |= (1 << GPIO_SDA_PIN))
#define IIC_SDA_CLR() (*(unsigned int *)GPIO0_DATA_ADD &= ~(1 << GPIO_SDA_PIN))
#define IIC_SCL_SET() (*(unsigned int *)GPIO0_DATA_ADD |= (1 << GPIO_SCL_PIN))
#define IIC_SCL_CLR() (*(unsigned int *)GPIO0_DATA_ADD &= ~(1 << GPIO_SCL_PIN))
#define IIC_SDA_GET() (((*(unsigned int *)GPIO0_DATA_ADD) >> GPIO_SDA_PIN) & 1)	//get the value of SDA pin

#define TYPE_DATA  0X40
#define TYPE_CMD   0

struct s_lcd_param{
	unsigned char data_type;	//
	unsigned char cmd;			//
	unsigned char data_buff[1024];	//data buffer

};
static struct s_lcd_param lcd_param;

static void lcd_gpio_init(void)
{
	unsigned int register_val = 0;

	//spi_cs1_mode,bit4:5=01 ,i2c_mode,bit20:21=01
	register_val = *(unsigned int *)GPIO1_MODE_ADDR;
	register_val = register_val & (~(3<<4)) & (~(3<<20));
	register_val = register_val | (1<<4) | (1<<20);
	*(unsigned int *)GPIO1_MODE_ADDR = register_val;
	IIC_RST_OUT();
	IIC_SDA_OUT();
	IIC_SCL_OUT();
	IIC_SDA_SET();
	IIC_SCL_CLR();
}
static void iic_start(void)
{
    // SDA 1->0 while SCL High
    IIC_SDA_SET();   
    IIC_SCL_SET();
    udelay(1);              
    IIC_SDA_CLR();
    udelay(1);                 
    IIC_SCL_CLR();
    udelay(1);
}

static void iic_stop(void)
{
	IIC_SCL_CLR(); 
    IIC_SDA_CLR(); 
    IIC_SCL_SET();                
    udelay(1);
    IIC_SDA_SET();
    udelay(1);
    IIC_SCL_CLR();
    udelay(1);
}
#define NOACK 1  //high = noack
#define ACK   0  //low  = ack
static unsigned char iic_read_ack(void)
{
    unsigned char rev_ack;
    
    IIC_SCL_CLR(); 
    IIC_SDA_CLR();
    IIC_SDA_IN();
	udelay(1);
    IIC_SCL_SET();
    udelay(1);
    rev_ack = (unsigned char)IIC_SDA_GET();  
    udelay(1);
    IIC_SCL_CLR();                    
    IIC_SDA_OUT();
    IIC_SDA_SET();
    udelay(1);
    return rev_ack;
}

static unsigned char iic_send_byte(unsigned char data)
{
    unsigned char rev_ack, i;
	IIC_SCL_CLR();
    for (i= 0 ; i< 8; i++){               
 		if (data & (0x80 >> i)){  // write data ,MSB first
       		IIC_SDA_SET();
 		}else {
       		IIC_SDA_CLR();
 		}
		udelay(1);
        IIC_SCL_SET();
        udelay(2);
        IIC_SCL_CLR();
        udelay(1);
    }
    rev_ack = iic_read_ack();
#if THE_DEBUG
    printk("drv_lcd.ko:(iic_send_byte) sendbyte=%x ack=%d\n",data, rev_ack);
#endif
    return rev_ack;
}

//返回EOS_OK=成功，EOS_FAIL=失败
static int iic_write_byte(unsigned char mode,unsigned char data)
{
    unsigned char rev_ack;
    iic_start();
    rev_ack = iic_send_byte(SLAVE_ADDR);	//send slave address and w/r w=0,r=1
    if(rev_ack == NOACK)
        return EOS_FAIL;
    rev_ack = iic_send_byte(mode);
    if(rev_ack == NOACK)
        return EOS_FAIL;
    rev_ack = iic_send_byte(data);
    if(rev_ack == NOACK)
        return EOS_FAIL;
    iic_stop();
    return EOS_OK;
}

static void iic_write_cmd(unsigned char cmd)
{
	iic_write_byte(TYPE_CMD,cmd);
#if THE_DEBUG
    printk("drv_lcd.ko:(iic_write_cmd) cmd = %x\n",cmd);
#endif
}

static void iic_write_dat(unsigned char data)
{
	iic_write_byte(TYPE_DATA,data);
}

/*
 * 填充LCD整屏数据
 */
#define LCD_MAX_POSX 128
#define LCD_MAX_POSY 64
#define ADD_OFFSET    2
static int lcd_fill_data_full(unsigned char * fill_data)
{
    unsigned char m, n;
    for (m = 0; m < (LCD_MAX_POSY/8); m++){
		iic_write_cmd(0x00 + ADD_OFFSET);		 //low column start address
		iic_write_cmd(0x10);					//high column start address
        iic_write_cmd(0xb0 + m);      //page0-page7
        iic_start();
        iic_send_byte(SLAVE_ADDR);
        iic_send_byte(TYPE_DATA); 
        for (n = 0; n < LCD_MAX_POSX; n++)
        {
            iic_send_byte(*(fill_data++));
        }
        iic_stop();
    }
	return EOS_OK;
}

static int lcd_init(void)
{
	IIC_RST_CLR();
	udelay(20);
	IIC_RST_SET();
	udelay(5);
	iic_write_cmd(0xAE); //Set Display Off
	iic_write_cmd(0xd5); //display divide ratio/osc. freq. mode
	iic_write_cmd(0x80); //
	iic_write_cmd(0xA8); //multiplex ration mode:63
	iic_write_cmd(0x3F);
	iic_write_cmd(0xD3); //Set Display Offset
	iic_write_cmd(0x00);
	iic_write_cmd(0x40); //Set Display Start Line
	iic_write_cmd(0xAD); //DC-DC Control Mode Set
	iic_write_cmd(0x8B); //8A:External 8b:Built-in DC-DC is used
	iic_write_cmd(0x32); //Set Pump voltage value
	iic_write_cmd(0xA1); //Segment Remap
	iic_write_cmd(0xC8); //Sst COM Output Scan Direction
	iic_write_cmd(0xDA); //common pads hardware: alternative
	iic_write_cmd(0x12);
	iic_write_cmd(0x81); //contrast control
	iic_write_cmd(0x40);
	iic_write_cmd(0xD9); //set pre-charge period
	iic_write_cmd(0x1f);
	iic_write_cmd(0xDB); //VCOM deselect level mode
	iic_write_cmd(0x40);
	iic_write_cmd(0xA4); //Set Entire Display On/Off
	iic_write_cmd(0xA6); //Set Normal Display
	iic_write_cmd(0xAF); //Set Display On

	return EOS_OK;
}

static int lcd_power_off(void)
{
    IIC_RST_CLR();
    return EOS_OK;
}

static void lcd_show_log(void)
{
    lcd_fill_data_full(ejoin_log);
}

static int lcd_open(struct inode *inode, struct file *file)
{
	return EOS_OK;
}

static int lcd_close(struct inode *inode, struct file *file)
{
	return EOS_OK;
}

static int lcd_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    return EOS_OK;
}

static int lcd_write(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//把显示内容复制到内核disp_buff中
	copy_from_user(&lcd_param, buff, 2);	//
	if(lcd_param.data_type == TYPE_DATA){
		copy_from_user(&lcd_param, buff, 1026);
		lcd_fill_data_full(lcd_param.data_buff);
	}else if(lcd_param.data_type == TYPE_CMD){
		iic_write_cmd(lcd_param.cmd);
	}
    
#if THE_DEBUG
        printk("drv_lcd.ko:(lcd_write) debug.\n");
#endif
	return EOS_OK;
}


static struct file_operations lcd_fops = {
	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    =  lcd_open,
	.release =  lcd_close,
	.read    =  lcd_read,
	.write	 =  lcd_write
};

static struct class             *screen_class;
static struct class_device	    *screen_class_dev;
static int major;

static int lcd_mod_init(void)
{
	major = register_chrdev(0, DEV_NAME, &lcd_fops); // login to the kernel
	screen_class = class_create(THIS_MODULE, DEV_NAME);
	screen_class_dev = device_create(screen_class, NULL, MKDEV(major, 0), NULL, DEV_NAME); /* /dev/lcdscreen */
    
	lcd_gpio_init();			//init the iic ports.
	lcd_init();    		//option the lcd.
	lcd_show_log();    	//show ejoin log.
	printk("Kevin:(drv_lcd.ko)(lcd_mod_init)| Create the device '%s',version(%s)\n", DEV_NAME, LCD_VERSION);
	return EOS_OK;
}

static void lcd_mod_exit(void)
{
	device_destroy(screen_class, MKDEV(major, 0));	//delete the node of the device
    class_destroy(screen_class);	//delete class
	unregister_chrdev(major, DEV_NAME); // logout device
	printk("Kevin:(drv_lcd.ko)(lcd_mod_exit)| Delete the device '%s',version(%s)\n", DEV_NAME, LCD_VERSION);
}

module_init(lcd_mod_init);
module_exit(lcd_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Wei");

