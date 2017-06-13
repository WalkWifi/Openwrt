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

//mifi v1.0	//20160803
#define STARTUP_VERSION "v1.0"

#define THE_DEBUG 0

#define GPIO1_MODE_ADD  0xB0000060
#define GPIO0_CTRL_ADD  0XB0000600
#define GPIO0_DATA_ADD  0XB0000620	//read or write data
#define GPIO1_CTRL_ADD  0XB0000604
#define GPIO1_DATA_ADD  0XB0000624

#define GPIO_NUM_MAX   46
#define GPIO_DIR_IN    0
#define GPIO_DIR_OUT   1

#define GPIO_BAT_POWER     11
#define GPIO_USB_EN  	   37  // REF_CLKO,enable the fast mode to charge

#define GPIO_G4_WAKEUP   27
#define GPIO_G4_RESET    28
#define GPIO_G4_POWER    29

#define GPIO_G3_WAKEUP     1
#define GPIO_G3_RESET      17
#define GPIO_G3_POWERON    16
#define GPIO_G3_POWEROFF   14


static int startup_gpio_init(unsigned int gpio, unsigned int dir, unsigned int val)
{
	volatile unsigned int * gpio_ctrl_add_tmp;
	volatile unsigned int * gpio_data_add_tmp;
	unsigned int gpio_val = 0;

	if((gpio > GPIO_NUM_MAX)||(dir > 1)||(val > 1)){	//valid
		printk("drv_startup.ko:(startup_gpio_init) parametric error!\n");
		return -1;
	}
	//gpio register address
	if(gpio < 32){
		gpio_ctrl_add_tmp = (volatile unsigned int *)GPIO0_CTRL_ADD;
		gpio_data_add_tmp = (volatile unsigned int *)GPIO0_DATA_ADD;
	}else{
		gpio -= 32;
		gpio_ctrl_add_tmp = (volatile unsigned int *)GPIO1_CTRL_ADD;
		gpio_data_add_tmp = (volatile unsigned int *)GPIO1_DATA_ADD;
	}
	
	gpio_val = *gpio_ctrl_add_tmp;
	gpio_val &= ~(1 << gpio);
	gpio_val |= (dir << gpio);
	*gpio_ctrl_add_tmp = gpio_val;
	
	gpio_val = *gpio_data_add_tmp;
	gpio_val &= ~(1 << gpio);
	gpio_val |= (val << gpio);
	*gpio_data_add_tmp = gpio_val;

	return 0;
}

static int startup_gpio_set(unsigned int gpio, unsigned int val)
{
	volatile unsigned int * gpio_data_add_tmp;
	unsigned int gpio_val = 0;

	if((gpio > GPIO_NUM_MAX)||(val > 1)){	//valid
		printk("drv_startup.ko:(startup_gpio_set) parametric error!\n");
		return -1;
	}
	//gpio register address
	if(gpio < 32){
		gpio_data_add_tmp = (volatile unsigned int *)GPIO0_DATA_ADD;
	}else{
		gpio -= 32;
		gpio_data_add_tmp = (volatile unsigned int *)GPIO1_DATA_ADD;
	}
	
	gpio_val = *gpio_data_add_tmp;
	gpio_val &= ~(1 << gpio);
	gpio_val |= (val << gpio);
	*gpio_data_add_tmp = gpio_val;

	return 0;
}

static int startup_mod_init(void)
{
	unsigned int regbuff = 0;
	regbuff = *(volatile unsigned int *)GPIO1_MODE_ADD;
	regbuff &= ~(3 << 0);	//GPIO0 set gpio mode
	regbuff |= (1 << 18);	//REF_CLKO GPIO_MODE
	regbuff &= ~(0x3 << 6);
	regbuff |= (0x1 << 6);	//GPIO1,i2s_mode = 01
	regbuff &= ~(3 << 2);
	regbuff |= (1 << 2);	//GPIO14.15.16.17,SPIS_MODE = 01
	regbuff &= ~(0x3 << 10);
	regbuff |= (0x1 << 10);	//GPIO27.28.29,SD_mode = 01
	*(volatile unsigned int *)GPIO1_MODE_ADD  = regbuff;


	startup_gpio_init(GPIO_BAT_POWER, GPIO_DIR_OUT, 1);     //battery enable
	startup_gpio_init(GPIO_USB_EN, GPIO_DIR_OUT, 1);		//enable fast charge

	startup_gpio_init(GPIO_G4_WAKEUP, GPIO_DIR_OUT, 0);     //G4,4G model enable,
	startup_gpio_init(GPIO_G4_RESET, GPIO_DIR_OUT, 1);      //no effect, the ppp module is not up,start the G4 has no effect
	startup_gpio_init(GPIO_G4_POWER, GPIO_DIR_OUT, 0);

	startup_gpio_init(GPIO_G3_WAKEUP, GPIO_DIR_OUT, 0);
	startup_gpio_init(GPIO_G3_RESET, GPIO_DIR_OUT, 1);
	startup_gpio_init(GPIO_G3_POWERON, GPIO_DIR_OUT, 1);
	startup_gpio_init(GPIO_G3_POWEROFF, GPIO_DIR_OUT, 1);

	printk("Kevin:(MIFI)(drv_startup.ko)(startup_mod_init)| startup option.version(%s)\n",STARTUP_VERSION);
	return 0;
}

static void startup_mod_exit(void)
{
	
	printk("Kevin:(MIFI)(drv_startup.ko)(startup_mod_exit)| startup option.version(%s)\n",STARTUP_VERSION);
}

module_init(startup_mod_init);
module_exit(startup_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Wei");


