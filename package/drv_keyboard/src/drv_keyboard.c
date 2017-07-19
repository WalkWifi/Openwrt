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

#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm/mach-ralink/surfboardint.h>
//The Kenel header file, include soc virtual address
#include <asm/mach-ralink/rt_mmap.h>


#define OLED_VESION 1.1	//2015.09.16

#define THE_DEBUG 0

#define RALINK_SYSCTL_ADDR        RALINK_SYSCTL_BASE    // system control
#define RALINK_REG_GPIOMODE        (RALINK_SYSCTL_ADDR + 0x60) //GPIO MODE

#define RALINK_PRGIO_ADDR        RALINK_PIO_BASE // Programmable I/O

#define RALINK_REG_PIO7140INT    (RALINK_PRGIO_ADDR + 0x60)//中断地址
#define RALINK_REG_PIO7140EDGE   (RALINK_PRGIO_ADDR + 0x64)//边沿触发方式地址
#define RALINK_REG_PIO7140RENA   (RALINK_PRGIO_ADDR + 0x68)//上升沿触发掩码
#define RALINK_REG_PIO7140FENA   (RALINK_PRGIO_ADDR + 0x6C)

#define  RALINK_REG_PIO7140DATA  (RALINK_PRGIO_ADDR + 0x70)    //数据地址

#define  RALINK_REG_PIO7140DIR     (RALINK_PRGIO_ADDR + 0x74)    //方向地址

#define  RALINK_IRQ_ADDR          RALINK_INTCL_BASE  
#define  RALINK_REG_INTENA        (RALINK_IRQ_ADDR   + 0x34)//enable 中断地址
#define  RALINK_REG_INTDIS        (RALINK_IRQ_ADDR   + 0x38)//disable 中断地址


//实际引脚是:GE2_R XD1,  对应GPIO67

 

//#include "key_driver.h"
#define KEY_DRIVER_NAME "key_driver"

//保存当前中断方式
static u32 ralink_gpio7140_intp = 0;
//保存当前边沿触发方式
static u32 ralink_gpio7140_edge = 0;

//gpio状态
typedef enum
{
    e_gpio_rising = 0,
    e_gpio_falling,
    e_gpio_edge_unknow
}e_gpio_edge_t;

//gpio对应信息
struct gpio_status
{
    int gpio_num;
    e_gpio_edge_t edge;
    u32 key_value;//键值传给用户空间
    struct timer_list timer;//定时器(用于去抖)
};

 

static struct gpio_status g_gpio67 = {67, e_gpio_edge_unknow, 0}; 

//生成一个等待队列头wait_queue_head_t,名字为key_waitq
/* 等待队列:
* 当没有按键被按下时，如果有进程调用key_driver_read函数，
* 它将休眠
*/
static DECLARE_WAIT_QUEUE_HEAD(key_waitq);

/* 中断事件标志, 中断服务程序将它置1，key_driver_read将它清0*/
static volatile int ev_press = 0;

 

/**
* 设置gpio60-gpio71模式为gpio.
*/
static void set_7160_gpio_mode(void)
{
    u32 gpiomode;
   
    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
    gpiomode |= (0x1<<10);
    *(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);
}

/**
* 设置gpio40-gpio71的数据方向.(按键设置gpio67为输入)
*/
static void set_7140_dir(int gpio,int dir)
{
    u32 gpiomode;

    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
    gpiomode &= ~(0x01<<(gpio-40));
    gpiomode |= (dir?0x01:0x0)<<(gpio-40);
   
    *(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(gpiomode);   
}

//读取gpio67引脚的数据
/*static int read_gpio_data(int gpio)
{
    unsigned long tmp;
   
    tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA));

    return (tmp&(0x1<<(gpio-40))) ? 1 : 0 ;
}*/

//设置pio enable interrupt
static void enable_intp(void)
{   
    //在rt_mmap.h头文件中定义RALINK_INTCTL_PIO ，即第六位控制pio中断
    //#define RALINK_INTCTL_PIO       (1<<6)
    *(volatile u32 *)(RALINK_REG_INTENA) = cpu_to_le32(RALINK_INTCTL_PIO);
}

//设置pio disable interrupt
static void disable_intp(void)
{
    //在rt_mmap.h头文件中定义RALINK_INTCTL_PIO ，即第六位控制pio中断
    //#define RALINK_INTCTL_PIO       (1<<6)
    *(volatile u32 *)(RALINK_REG_INTDIS) = cpu_to_le32(RALINK_INTCTL_PIO);
}

//set Edge Interrupt
static void gpio_reg_irq(int irq)
{
    unsigned long tmp;
   
    tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140RENA));
    tmp |= (0x1 << (irq-40));
    *(volatile u32 *)(RALINK_REG_PIO7140RENA) = cpu_to_le32(tmp);
    /*tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140FENA));
    tmp |= (0x1 << (irq-40));
    *(volatile u32 *)(RALINK_REG_PIO7140FENA) = cpu_to_le32(tmp);*/
}

//先保存当前中断及触发寄存器的值,再清空
static void ralink_gpio7140_save_clear_intp(void)
{
    //保存当前中断寄存器数据
    ralink_gpio7140_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140INT));
    //保存当前边沿触发方式
    ralink_gpio7140_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140EDGE));
    *(volatile u32 *)(RALINK_REG_PIO7140INT) = cpu_to_le32(0xFFFFFFFF);
    *(volatile u32 *)(RALINK_REG_PIO7140EDGE) = cpu_to_le32(0xFFFFFFFF);
}

//扫描时候是gpio67被按下
static int scan_gpio_num(void)
{
    if(!(ralink_gpio7140_intp & (1<<(g_gpio67.gpio_num - 40))))
    {
        printk("Have no key pressed...\n");
        return -1;
    }
    if (ralink_gpio7140_edge & (1<<(g_gpio67.gpio_num - 40))) {
        printk("set gpio value..\n");
            g_gpio67.edge = e_gpio_rising;
            g_gpio67.key_value = 1;
                //上升沿才有效，具体需要根据硬件设计。
    }
    else {
        printk("Have no edag...\n");
        return -1;
    }
    return 0;
}


//打开设备
static int key_driver_open(struct inode *inode, struct file *file)
{
    set_7160_gpio_mode();            //set RGMII2_GPIO_MODE to gpio mode.pro.p38
    set_7140_dir(0x01<<(67-40),0);    //set gpio60-gpio63 to gpin.
    enable_intp();        //set pio enable interrupt
   
    gpio_reg_irq(67);     //set Edge Interrupt
    return 0;
}

//关闭设备
static int key_driver_close(struct inode *inode, struct file *file)
{
    disable_intp();
    //禁止中断
    return 0;
}

static int key_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    unsigned long err;

    if(!ev_press) {
        if(filp->f_flags & O_NONBLOCK)
            return -EAGAIN;
        else//由wake_up_interruptible等待中断
            wait_event_interruptible(key_waitq, ev_press);
    }
    ev_press = 0;
    err = copy_to_user(buff, (const void *)&g_gpio67.key_value, sizeof(g_gpio67.key_value));

    g_gpio67.edge = e_gpio_edge_unknow;
    g_gpio67.key_value = 0x0;
   
    return err ? -EFAULT : sizeof(g_gpio67.key_value);
}


static unsigned int key_driver_poll( struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;

    /*把调用poll 或者select 的进程挂入队列，以便被驱动程序唤醒*/
    /*需要注意的是这个函数是不会引起阻塞的*/
    poll_wait(file, &key_waitq, wait);
    if(ev_press)
        mask |= POLLIN | POLLRDNORM;
    return mask;
}

static struct file_operations key_fops =
{
    .owner       = THIS_MODULE,
    .open        = key_driver_open,
    .release     = key_driver_close,
    .read        = key_driver_read,
    .poll        = key_driver_poll,
};

static struct miscdevice key_misc =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = KEY_DRIVER_NAME,
    .fops = &key_fops,
};

static void key_timer_handler(unsigned long data)
{
    ev_press = 1; /*设置中断标志为1*/
    wake_up_interruptible(&key_waitq); /*唤醒等待队列*/
    del_timer(&(g_gpio67.timer));
   
    printk("interrupt wake up...\n");
       
}


//中断处理函数
/*关于中断处理函数的返回值:中断程序的返回值是一个特殊类型—irqreturn_t。
中断程序的返回值却只有两个: IRQ_NONE和IRQ_HANDLED。*/
static irqreturn_t ralink_key_interrupt(int irq, void *irqaction)
{
    int ret;

    printk("interrupt handler...\n");
   
    //先保存当前中断及触发寄存器的值,再清空
    ralink_gpio7140_save_clear_intp();

    //查看是否是gpio67被按下
    ret = scan_gpio_num();

    //printk("Int func,gpio num %d.jiffies %ld,HZ %d\n",num,jiffies,HZ);
    if(ret < 0)
        return IRQ_RETVAL(IRQ_NONE);

    //设置定时器10ms
    init_timer(&(g_gpio67.timer));
    g_gpio67.timer.expires = jiffies + HZ/100;//消抖时间为10ms
    g_gpio67.timer.function = &key_timer_handler;
    add_timer(&(g_gpio67.timer));
   
    return IRQ_RETVAL(IRQ_HANDLED);
}

 

//初始化
static int __init key_driver_init(void)
{
    int ret;
    /*注册中断请求
     中断号:SURFBOARDINT_GPIO
     中断处理函数:ralink_key_interrupt
     中断属性(方式):上升沿触发
     使用此中断的设备:gpio_key
    */
    ret = request_irq(SURFBOARDINT_GPIO, ralink_key_interrupt,
                     /*IRQF_DISABLED |*/ IRQ_TYPE_EDGE_RISING, "gpio_key", NULL);
    if (ret)
        return ret;

    ret = misc_register(&key_misc);//初始化设备
    printk("key_driver_init OK!\n");
    return ret;
}

//退出
static void __exit key_driver_exit(void)
{
    int ret;

    free_irq(SURFBOARDINT_GPIO,NULL);//注销中断
   
    ret = misc_deregister(&key_misc);//注销设置
    if(ret < 0)
        printk("key_driver_exit error.\n");
    printk("key_driver_exit.\n");
}

module_init(key_driver_init);
module_exit(key_driver_exit);

MODULE_LICENSE("GPL");

