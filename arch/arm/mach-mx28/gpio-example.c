#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/module.h> 
#include <mach/pinctrl.h>

#include "regs-pinctrl.h"

#define PINCTRL_BASE_ADDR IO_ADDRESS(PINCTRL_PHYS_ADDR)

static int mxs_gpio_request(struct gpio_chip *chip, unsigned int pin) 
{
	void __iomem *addr = PINCTRL_BASE_ADDR;

	pin += 4;
	__raw_writel(0x3 << pin * 2, addr + HW_PINCTRL_MUXSEL6_SET);		/* set as GPIO		*/

	return 0;
}

static int mxs_gpio_free(struct gpio_chip *chip, unsigned int pin)
{
        void __iomem *addr = PINCTRL_BASE_ADDR;

        pin += 4;
        __raw_writel(0x3 << pin * 2, addr + HW_PINCTRL_MUXSEL6_CLR);            /* clear as GPIO          */

        return 0;
}


static int mxs_gpio_input(struct gpio_chip *chip, unsigned int index)
{
	 void __iomem *base = PINCTRL_BASE_ADDR;

	index += 4;
	__raw_writel(1 << index, base + HW_PINCTRL_DOE3_CLR);

	return 0;
}

static int mxs_gpio_get(struct gpio_chip *chip, unsigned int index)
{
	unsigned int data;
	void __iomem *base = PINCTRL_BASE_ADDR;

	index += 4;
	data = __raw_readl(base + HW_PINCTRL_DIN3);

	return data & (1 << index);
}

static int mxs_gpio_output(struct gpio_chip *chip, unsigned int index, int v)
{
	void __iomem *base = PINCTRL_BASE_ADDR;

	index += 4;
	__raw_writel(1 << index, base + HW_PINCTRL_DOE3_SET);		/* 设置为输出工作模块		*/
	
	if (v) {	 	/* 当v为非0时， 设置GPIO输出高电平   	*/
		__raw_writel(1 << index, base + HW_PINCTRL_DOUT3_SET);
	} else {   		/* 当v为0时，设置GPIO输出低电平 		*/
		__raw_writel(1 << index, base + HW_PINCTRL_DOUT3_CLR);
	}

	return 0;
}

static void mxs_gpio_set(struct gpio_chip *chip, unsigned int index, int v)
{
	void __iomem *base = PINCTRL_BASE_ADDR;

	index += 4;
	if (v) {	 /* 设置GPIO输出高电平   	*/
		__raw_writel(1 << index, base + HW_PINCTRL_DOUT3_SET);
	} else { 	/* 设置GPIO输出低电平 		*/
		__raw_writel(1 << index, base + HW_PINCTRL_DOUT3_CLR);
	}
}

struct gpio_chip mx28_gpio_chip = {
	.label		    	= "example_gpio",
	.owner			= THIS_MODULE,
	.base		    	= 160,
	.ngpio			= 2,
	.request	    	= mxs_gpio_request,
	.free		    	= mxs_gpio_free,
	.direction_input 	= mxs_gpio_input,
	.get		    	= mxs_gpio_get,
	.direction_output	= mxs_gpio_output,
	.set		    	= mxs_gpio_set,
	.exported	    	= 1,
};

static int __init mx28_gpio_init(void)
{
	int ret = 0;

	ret = gpiochip_add(&mx28_gpio_chip);
	if (ret) {
		printk("add example gpio faile:%d \n", ret);
		goto out;
	}
	printk("add example gpio success... \n");
out:
	return ret;
}

static void __exit mx28_gpio_exit(void)
{
	gpiochip_remove(&mx28_gpio_chip);
	printk("remove example gpio... \n");
}

module_init(mx28_gpio_init);
module_exit(mx28_gpio_exit);

MODULE_AUTHOR("chm <chenhaiman@zlg.cn>");
MODULE_DESCRIPTION("gpio Test driver");
MODULE_LICENSE("GPL");

