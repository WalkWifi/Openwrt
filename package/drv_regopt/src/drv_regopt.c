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

#define REGOPT_DEBUG 0
#define REGOPT_VERSION "2.0"	//2016.01.20
#define DEV_NAME    "regopt"

static struct class             *regopt_class;
static struct class_device	*regopt_class_dev;

struct s_reg_param{
	volatile unsigned int reg_add;	//寄存器物理地址
	volatile unsigned int reg_val;	//寄存器值
	volatile unsigned int *vm_add;	//寄存器映射地址
};

/*user 和kenel交互数据的结构体，
 * reg_add 寄存器地址
 * reg_val 寄存器的值
 * vm_add  映射到虚拟地址
 */
static struct s_reg_param reg_param = {
	.reg_add = 0,
	.reg_val = 0,
	.vm_add  = NULL,
};

static int regopt_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int regopt_close(struct inode *inode, struct file *file)
{
	return 0;
}

static int regopt_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//一定要先设置用户区的buff（即应用层read函数到第二个参数），就是要修改到物理寄存器地址
	copy_from_user(&reg_param, buff, 4);	//物理地址4字节
	reg_param.vm_add = (volatile unsigned int *)ioremap(reg_param.reg_add, 0x4);	//映射寄存器
	reg_param.reg_val = *(reg_param.vm_add);	//取出寄存器
	copy_to_user(buff,&reg_param, 8);	//物理地址和值，2个4字节
	iounmap(reg_param.vm_add);	//取消映射
#if REGOPT_DEBUG
        printk("drv_regopt.ko:(regopt_read) read_add: 0x%x = 0x%x\n", reg_param.reg_add, reg_param.reg_val);
#endif
    	return 0;
}

static int regopt_write(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	//把要修改的参数复制到内核reg_param中
	copy_from_user(&reg_param, buff, 8);	//物理地址和值，2个4字节
	reg_param.vm_add = (volatile unsigned int *)ioremap(reg_param.reg_add, 0x4);	//映射寄存器
	*(reg_param.vm_add) = reg_param.reg_val;
	iounmap(reg_param.vm_add);	//取消映射
#if REGOPT_DEBUG
        printk("drv_regopt.ko:(regopt_read) write_add: 0x%x = 0x%x\n", reg_param.reg_add, reg_param.reg_val);
#endif
	return 0;
}

static struct file_operations regopt_fops = {
	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    =  regopt_open,
	.release =  regopt_close,
	.read    =  regopt_read,
	.write	 =  regopt_write   
};

static int major;
static int regopt_mod_init(void)
{
	major = register_chrdev(0, DEV_NAME, &regopt_fops); // 注册, 告诉内核
	regopt_class = class_create(THIS_MODULE, DEV_NAME);
	regopt_class_dev = device_create(regopt_class, NULL, MKDEV(major, 0), NULL, DEV_NAME); /* /dev/regopt */
	printk("Kevin:(drv_regopt.ko)(regopt_mod_init)| Create the device '%s',version(%s)\n", DEV_NAME, REGOPT_VERSION);
	return 0;
}

static void regopt_mod_exit(void)
{
	device_destroy(regopt_class, MKDEV(major, 0));	//删除设备节点
    	class_destroy(regopt_class);	//删除类
	unregister_chrdev(major, DEV_NAME); // 注销设备
	printk("Kevin:(drv_regopt.ko)(regopt_mod_exit)| Delete the device '%s',version(%s)\n", DEV_NAME, REGOPT_VERSION);
}

module_init(regopt_mod_init);
module_exit(regopt_mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Wei");
