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

#define REGOPT_DEBUG 0
#define DEV_NAME    "oledscreen"
#define DEV_ADDR    0x7a

#define GPIO1_MODE_ADDR 0x10000060
#define I2C_ADDR_BASE 0x10000900
#define I2C_SM0CFG0   *(volatile unsigned int *)(i2c_base + 0x08)
#define I2C_SM0DOUT   *(volatile unsigned int *)(i2c_base + 0x10)
#define I2C_SM0DIN    *(volatile unsigned int *)(i2c_base + 0x14)
#define I2C_SM0ST     *(volatile unsigned int *)(i2c_base + 0x18)
#define I2C_SM0AUTO   *(volatile unsigned int *)(i2c_base + 0x1C)
#define I2C_SM0CFG1   *(volatile unsigned int *)(i2c_base + 0x20)
#define I2C_SM0CFG2   *(volatile unsigned int *)(i2c_base + 0x28)
#define I2C_SM0CTL0   *(volatile unsigned int *)(i2c_base + 0x40)
#define I2C_SM0CTL1   *(volatile unsigned int *)(i2c_base + 0x44)
#define I2C_SM0D0     *(volatile unsigned int *)(i2c_base + 0x50)
#define I2C_SM0D1     *(volatile unsigned int *)(i2c_base + 0x54)
#define I2C_PINTEN    *(volatile unsigned int *)(i2c_base + 0x5C)

static volatile void __iomem *i2c_base;
static volatile void __iomem *gpio1_mode;

static struct class             *screen_class;
static struct class_device	*screen_class_dev;
static int major;

struct s_oled_param{
	volatile unsigned char disp_buff[1024];	//oled display buffer
	volatile unsigned int reg_val;	//寄存器值
	volatile unsigned int *vm_add;	//寄存器映射地址
};

/*user 和kenel交互数据的结构体，
 * reg_add 寄存器地址
 * reg_val 寄存器的值
 * vm_add  映射到虚拟地址
 */
static struct s_oled_param oled_param;

void iic_write_byte(unsigned char unAddr,unsigned char ucData)
{
	//f_nGetACK = 0;

	// Send control byte
	//Send address
	I2C_SM0DOUT = unAddr;
	I2C_SM0AUTO = 0;				// Resumes IIC operation.
	//while(f_nGetACK == 0);// Wait ACK
	//f_nGetACK = 0;
        udelay(25);

	// Send data 
	I2C_SM0DOUT = ucData;
	I2C_SM0AUTO = 0;				// Resumes IIC operation.
	//while(f_nGetACK == 0);// Wait ACK
	///f_nGetACK = 0;
        udelay(25);

	// End send
	//rIICSTAT = 0xd0;			// Stop Master Tx condition
	//rIICCON = 0xaf;				// Resumes IIC operation.
	udelay(100);				// Wait until stop condtion is in effect.
}

static int oled_open(struct inode *inode, struct file *file)
{
        int result;
	// Initialize iic
	//rIICCON = 0xaf;					// Enable ACK, interrupt, SET IICCLK=MCLK/16
	//rIICSTAT = 0x10;				// Enable TX/RX 
	//rGPECON =(rGPECON&((~0xf)<<28))+(0xa<<28);

        
        I2C_PINTEN = 1; //enable interrupt
	//result = request_irq (IRQ_IIC, iic_int_24c04, IRQF_DISABLED, DEV_NAME, NULL);
	//if (result) {
	//	printk(KERN_INFO "I2C: can't get assigned irq\n");
	//}
        iic_write_byte(0xAE, 0XAE);
	return 0;
}

static int oled_close(struct inode *inode, struct file *file)
{
        
	//free_irq(IRQ_IIC, NULL);
	printk("I2C closed\n");
	return 0;
}

static int oled_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    	return 0;
}

static int oled_write(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//把显示内容复制到内核disp_buff中
	//copy_from_user(oled_param.disp_buff, buff, 8);	//物理地址和值，2个4字节
        
#if REGOPT_DEBUG
        printk("0101\n");
#endif
	return 0;
}

static int oled_ioctl(struct inode *inode,struct file *file,unsigned int cmd,unsigned long arg)
{	
	return 0;	
}

static struct file_operations oled_fops = {
	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    =  oled_open,
	.release =  oled_close,
	.read    =  oled_read,
	.write	 =  oled_write
        //.ioctl   =  oled_ioctl  
};

static int oled_init(void)
{
	major = register_chrdev(0, DEV_NAME, &oled_fops); // 注册, 告诉内核
	screen_class = class_create(THIS_MODULE, DEV_NAME);
	screen_class_dev = device_create(screen_class, NULL, MKDEV(major, 0), NULL, DEV_NAME); /* /dev/oledscreen */
        //address map
        i2c_base   = ioremap(I2C_ADDR_BASE, 0x68);
	gpio1_mode = ioremap(GPIO1_MODE_ADDR, 0x4);
        
        //config i2c register
        *(volatile int *)gpio1_mode &=~(0x3 << 20);     //i2c_mode 
        I2C_SM0CFG0 = DEV_ADDR; //
        I2C_SM0CFG1 = 0; //
        I2C_SM0CFG2 = 1; //AUTO MODE
        I2C_SM0CTL0 = 0xC8800c; //div=200,
        I2C_SM0CTL1 = 0;
        I2C_PINTEN  = 0;
	printk("oledscreen_init\n");
	return 0;
}

static void oled_exit(void)
{
	device_destroy(screen_class, MKDEV(major, 0));	//删除设备节点
    	class_destroy(screen_class);	//删除类
	unregister_chrdev(major, DEV_NAME); // 注销设备
        iounmap(i2c_base);
        iounmap(gpio1_mode);
	printk("oledscreen_exit\n");
}

module_init(oled_init);
module_exit(oled_exit);


MODULE_LICENSE("GPL");

