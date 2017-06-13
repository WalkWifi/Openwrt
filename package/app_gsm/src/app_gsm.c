#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

#include <sys/ioctl.h>
#include <linux/serial.h>

//usleep(n) //n微秒
//Sleep（n）//n毫秒
//sleep（n）//n秒

#define UART1_DEBUG 1

#define FALSE -1
#define TRUE   0

#define G3_WAKEUP_IO   1

#define G3_RESET_IO    17
#define G3_POWERON_IO  16
#define G3_POWEROFF_IO 14


#define G4_WAKEUP_IO 27
#define G4_RESET_IO 28
#define G4_POWER_IO 29

#define GPIO1_MODE  0X10000060  //系统模块模式设置

#define GPIO_CTRL_0 0X10000600	//0=in,1=out
#define GPIO_DATA_0 0X10000620  //io data
#define GPIO_DSET_0 0X10000630	//1=set
#define GPIO_DCLR_0 0X10000640	//1=clr

#define UART1_NAME "/dev/ttyS2"
#define REG_DEV_NAME "/dev/regopt"


/*寄存器配置
  add=物理地址
  val=将要写入的值
  mask=val的有效位，1=有效
 */
static int reg_option(unsigned int add, unsigned int val, unsigned int mask)
{
	int fd;
	unsigned int reg_val[2];
	fd = open(REG_DEV_NAME , O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd < 0){
		printf("can't open!\n");
		return FALSE;
	}
	//配置rst IO口为输出，默认
	reg_val[0] = add;
	read(fd, reg_val, 8);
	reg_val[1] &= ~mask;	//清除需要设置的位
	reg_val[1] |= val;		//设置
	write(fd, reg_val, 8);

	close(fd);
	return TRUE;

}



/*
 *这个函数初始化SIM卡到控制端口
 *GPIO#11-DTR（RST，0复位）；GPIO#12-DCD（OE，1=高阻，0=打开）；
 *参数：gpio--端口号，dir--in=0/out=1，value--电平
 */
static int gpio_init(unsigned char gpio)
{
	int fd;
	unsigned int reg_val[2];
	fd = open(REG_DEV_NAME , O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd < 0){
		printf("can't open!\n");
		return FALSE;
	}
	//配置IO口为输出，默认0=输入
	reg_val[0] = GPIO_CTRL_0;
	read(fd, reg_val, 8);
	reg_val[1] |= 1 << gpio;
	write(fd, reg_val, 8);
        //配置输出高电平
	reg_val[0] = GPIO_DATA_0;
        read(fd, reg_val, 8);
        reg_val[1] |= 1 << gpio;
        write(fd, reg_val, 8);
	close(fd);
	return TRUE;
	
}

static int gpio_output(unsigned char io, unsigned char val)
{
	int fd;
	unsigned int reg_val[2];
	if((io > 31) || (val > 1)){
		printf("param is wrong!\n");
		return FALSE;
	}
	fd = open(REG_DEV_NAME , O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd < 0){
		printf("can't open!\n");
		return FALSE;
	}
	//输出0/1
        if(val == 0){
                reg_val[0] = GPIO_DATA_0;
                read(fd, reg_val, 8);
	        reg_val[1] &= ~(1 << io);
	        write(fd, reg_val, 8);
        }else if(val == 1){
	        reg_val[0] = GPIO_DATA_0;
                read(fd, reg_val, 8);
	        reg_val[1] |= 1 << io;
	        write(fd, reg_val, 8);
        }
	
	close(fd);
	return TRUE;
}

int main(int argc, char **argv)
{
	int fd,fd_temp;                            //文件描述符
	int err;                           //返回调用函数的状态
	int len;                        
	int i;
	char rcv_buf[100];       
	char send_buf[20]="AT+IPR?";
        send_buf[7]=0x0D;
        send_buf[8]=0x0A;
        
	if(argc < 3){
	      printf("Usage: %s 3g/4g 0/1\n",argv[0]);
	      return FALSE;
	}
    if(0 == strcmp(argv[1],"4g")){
    	if((0 == strcmp(argv[2],"0"))||(0 == strcmp(argv[2],"1"))){
	        gpio_init(G4_RESET_IO);
	        gpio_init(G4_POWER_IO);
	        gpio_init(G4_WAKEUP_IO);
	        gpio_output(G4_WAKEUP_IO, 0);
	        gpio_output(G4_RESET_IO, 1);
			gpio_output(G4_POWER_IO, 0);
	        usleep(500000);
	        if(0 == strcmp(argv[2],"1")){
	        	gpio_output(G4_POWER_IO, 1);
	        }
	    }

	}else if(0 == strcmp(argv[1],"3g")){     
		if((0 == strcmp(argv[2],"0"))||(0 == strcmp(argv[2],"1"))){           
	        //set the reset and power pin of the g4 module out
	        reg_option(GPIO1_MODE, 0x1 << 6, 0x3<<6);     //GPIO1,i2s_mode = 01
			reg_option(GPIO1_MODE, 0x1 << 2, 0x3<<2);     //GPIO14.15.16.17,SPIS_MODE = 01
	        gpio_init(G3_RESET_IO);
	        gpio_init(G3_POWERON_IO);
	        gpio_init(G3_POWEROFF_IO);
	        gpio_init(G3_WAKEUP_IO);
	        gpio_output(G3_WAKEUP_IO, 0);                
	        gpio_output(G3_POWERON_IO, 1);
	        gpio_output(G3_POWEROFF_IO, 0);                            
	        gpio_output(G3_RESET_IO, 0);                
	        usleep(200000);
	        gpio_output(G3_POWEROFF_IO, 1);
	        gpio_output(G3_RESET_IO, 1);
	        usleep(500000);
	        if(0 == strcmp(argv[2],"1")){
		        gpio_output(G3_POWERON_IO, 0);
		        usleep(500000);
		        gpio_output(G3_POWERON_IO, 1);
		    }
	    }
    }
	return 0;
}
  
/***************************  End  **************************************/







