/*********************************************Copyright (c)***********************************************
**                                Guangzhou ZLG MCU Technology Co., Ltd.
**
**                                        http://www.zlgmcu.com
**
**      广州周立功单片机科技有限公司所提供的所有服务内容旨在协助客户加速产品的研发进度，在服务过程中所提供
**  的任何程序、文档、测试结果、方案、支持等资料和信息，都仅供参考，客户有权不使用或自行参考修改，本公司不
**  提供任何的完整性、可靠性等保证，若在客户使用过程中因任何原因造成的特别的、偶然的或间接的损失，本公司不
**  承担任何责任。
**                                                                        ——广州周立功单片机科技有限公司
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               lradc.c
** Last modified date:      2013年11月28日15:57:38
** Last version:            
** Descriptions:           
**
**--------------------------------------------------------------------------------------------------------
** Created by:              周华
** Created date:            2013年11月28日15:57:44
** Version:           	    v1.0
** Descriptions:            ADC驱动，读取
**--------------------------------------------------------------------------------------------------------
** Modified by:        
** Modified date:      
** Version:            
** Descriptions:        ADC Vref = Bandgap Reference(channel 14) = 1.85V
**			如果电压大于1.85V，需要开启硬件除2
**			ADC为12位
**			每个寄存器都有四个，分别用来 r/w 、set 、clear 、trogger 
**                          名称类似 HW_LRADC_CTRL0、HW_LRADC_CTRL0_SET、
**                          HW_LRADC_CTRL0_CLR、HW_LRADC_CTRL0_TOG       
**  HW_LRADC_CTRL0      SFTRST(31)  CLKGATE(30) SCHEDULE(7~0)   
**  HW_LRADC_CTRL1      CHn_IRQ_EN(23~16)   CHn_IRQ(7~0)        主要为触屏、ADC中断
**  HW_LRADC_CTRL2      CHn_DIV/2(31~24)                        主要为温度控制
**  HW_LRADC_CTRL3      DISCARD(25、24) CYCLE_TIME(9、8)HIGH_TIME(5/4)DELAY_CLK(1)INVERT_CLK(0)忽略、时钟
**  HW_LRADC_CTRL4      4*8 channels 0-16 channel selector
**                      
*********************************************************************************************************/

#include<linux/module.h>                                                /* module                       */
#include<linux/fs.h>                                                    /* file operation               */
#include<asm/uaccess.h>                                                 /* get_user()                   */
#include<linux/miscdevice.h>                                            /* miscdevice                   */
#include<asm/io.h>                                                      /* ioctl                        */
#include <mach/regs-lradc.h>						/* #define                      */


static void __iomem *adc_base = NULL;


/*********************************************************************************************************
 文件操作接口
 被注释的语句在没有使用触摸屏驱动时，需要开启	
*********************************************************************************************************/
static int adc_open (struct inode *inode, struct file *fp) 
{
// 	long time = 10000;	

	try_module_get(THIS_MODULE);    
//	writel(0x40000000, adc_base + HW_LRADC_CTRL0_CLR);
//	writel(0x80000000, adc_base + HW_LRADC_CTRL0_SET);              /* 复位                         */
//	while(readl(adc_base + HW_LRADC_CTRL0) & 0x80000000);           /* 等待复位结束                 */
//        while(time--);                      
	writel(0xC0000000, adc_base + HW_LRADC_CTRL0_CLR);              /* 开启CLK，关闭复位模式        */
	writel(0x18430000, adc_base + HW_LRADC_CTRL1_CLR);              /* 关闭0-1-6通道及按键中断      */ 
	writel(0x30000000, adc_base + HW_LRADC_CTRL3_SET);              /* 忽略上电前三次采集数据       */     
	writel(0x76543210, adc_base + HW_LRADC_CTRL4);                  /* 设置对应的数据存放位置       */
	
	return 0;
}
static int adc_release (struct inode *inode, struct file *fp)
{
	module_put(THIS_MODULE);

	return 0;
}
/*********************************************************************************************************
**   cmd: 
**	10:CH0     	11:CH1    	16:CH6  	 17:Vbat(内部除4)	  
**      20:CH0(开启除2) 21:CH1(开启除2) 26::CH6(开启除2) 
**
**   被注释部分为3.0以上内核函数定义形式
*********************************************************************************************************/

//static int adc_ioctl(struct file *fp, struct file *flip, unsigned int cmd, unsigned long arg)
static int adc_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
    	int iRes;
	int iTmp;
	
	if(cmd <10 || cmd >27){
		return -1;
	}

	iTmp = cmd % 10;
   		
	if(cmd == 20){
		writel(0x01000000, adc_base + HW_LRADC_CTRL2_SET);
	}
	if(cmd == 10){	
		writel(0x01000000, adc_base + HW_LRADC_CTRL2_CLR);
	}
        if(cmd == 21){
                writel(0x02000000, adc_base + HW_LRADC_CTRL2_SET);
        }
	if(cmd == 11){	
		writel(0x02000000, adc_base + HW_LRADC_CTRL2_CLR);
	}
        if(cmd == 26){	
                writel(0x40000000, adc_base + HW_LRADC_CTRL2_SET);
        }
	if(cmd == 16){	
		writel(0x40000000, adc_base + HW_LRADC_CTRL2_CLR);
	}
	
	writel(0x00000001 << iTmp, adc_base + HW_LRADC_CTRL0_SET);
        
    	while(!( readl(adc_base + HW_LRADC_STATUS) & ((1 << 16) << iTmp) )); 
    		
	iRes = readl(adc_base + HW_LRADC_CHn(iTmp));

	iRes = iRes & 0x0003FFFF;	
    
   	copy_to_user((void *)arg, (void *)(&iRes), sizeof(int));
    
    	return 0;
}
/*********************************************************************************************************
Device Struct
*********************************************************************************************************/
struct file_operations adc_fops = 
{
	.owner		= THIS_MODULE,
	.open		= adc_open,
	.release	= adc_release,
    	.ioctl      	= adc_ioctl,
};
static struct miscdevice adc_miscdev = 
{
	.minor	        = MISC_DYNAMIC_MINOR,
   	.name	        = "mxs-adc",
    	.fops	        = &adc_fops,
};
/*********************************************************************************************************
Module Functions
*********************************************************************************************************/
static int __init adcModule_init (void)
{
    	int iRet=0;
    	printk("\nadc module init!\n");
	
	adc_base = ioremap(0x80050000, 0x180*4);    
	
    	iRet = misc_register(&adc_miscdev);
	if (iRet) {
		printk("register failed!\n");
	} 
	return iRet;
}
static void __exit adcModule_exit (void)                                /* warning:return void          */
{
	printk("\nadc module exit!\n");
	misc_deregister(&adc_miscdev);
}
/*********************************************************************************************************
Driver Definitions
*********************************************************************************************************/
module_init(adcModule_init);
module_exit(adcModule_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("admin@9crk.com");
MODULE_DESCRIPTION("EasyARM283 By zhouhua");
/*********************************************************************************************************
End File
*********************************************************************************************************/


