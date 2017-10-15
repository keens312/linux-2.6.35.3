/*
   GPIO Driver driver for EasyARM-iMX283
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/bcd.h>
#include <linux/capability.h>
#include <linux/rtc.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/slab.h>


#include <../arch/arm/mach-mx28/mx28_pins.h>
#include "gpio.h"

struct gpio_info {
	u32  	 	   pin;
	char 	 	   pin_name[20];
	struct miscdevice *pmiscdev;
};

static struct gpio_info *gpio_info_file[255];

static struct gpio_info all_gpios_info[] ={
	/* J1 in imx287 */
	{PINID_AUART3_RTS,     "gpio3-15",  	NULL}, /* SIM800G_ON */
	{PINID_AUART3_CTS,     "gpio3-14",  	NULL}, /* nWAKE */
	{PINID_SAIF1_SDATA0,   "gpio3-26",   	NULL}, /* UART_DTR */
	{PINID_AUART2_CTS,   "gpio3-10",   	NULL}, /* RESET */
	{PINID_AUART2_RTS,   "gpio3-11",   	NULL}, /* W_DIS */
	{PINID_SSP2_MISO,   "gpio2-18",   	NULL}, /* UART_RI */
	{PINID_SAIF0_MCLK,   "gpio3-20",   	NULL}, /* UART_RI */
	{PINID_SAIF0_LRCLK,   "gpio3-21",   	NULL}, /* UART_RI */

	{0,		   "",          NULL}   //the end
};

/*--------------------------------------------------------------------------------------------------------*/
static int gpio_open(struct inode *inode, struct file *filp);
static int  gpio_release(struct inode *inode, struct file *filp);
ssize_t gpio_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos);
static int gpio_ioctl(struct inode *inode,struct file *flip,unsigned int command,unsigned long arg);
static int gpio_init(void);
static void gpio_exit(void);

/*--------------------------------------------------------------------------------------------------------*/

static int gpio_open(struct inode *inode, struct file *filp)
{
	struct gpio_info *gpio_info_tmp;
	u32 minor = iminor(inode);

	gpio_info_tmp = gpio_info_file[minor];

	gpio_free(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));
	if (gpio_request(MXS_PIN_TO_GPIO(gpio_info_tmp->pin), gpio_info_tmp->pin_name)) {
		printk("request %s gpio faile \n", gpio_info_tmp->pin_name);
		return -1;
	}

	filp->private_data = gpio_info_file[minor];

	return 0;
}

static int  gpio_release(struct inode *inode, struct file *filp)
{
	struct gpio_info *gpio_info_tmp = (struct gpio_info *)filp->private_data;

	gpio_free(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));

	return 0;
}


ssize_t gpio_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct gpio_info *gpio_info_tmp = (struct gpio_info *)filp->private_data;
	char data = '0';

	copy_from_user(&data, buf, sizeof(data));
	data = data - '0';

    if (data == 1 || data == 0) {
            gpio_direction_output(MXS_PIN_TO_GPIO(gpio_info_tmp->pin), data);
    }

	return count;
}


static ssize_t gpio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct gpio_info *gpio_info_tmp = (struct gpio_info *)file->private_data;
	char data = '0';

	gpio_direction_input(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));
	data = gpio_get_value(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));	
	
	data = data ? '1' : '0';

	copy_to_user(buf, &data, sizeof(data));
	
    return sizeof(data);
}

static int gpio_ioctl(struct inode *inode,struct file *flip,unsigned int command,unsigned long arg)
{
	struct gpio_info *gpio_info_tmp = (struct gpio_info *)flip->private_data;
	int  data = 0;
	
	switch (command) {
	case SET_GPIO_HIGHT: 
		gpio_direction_output(MXS_PIN_TO_GPIO(gpio_info_tmp->pin), 1);
		break;
	
	case SET_GPIO_LOW:
		gpio_direction_output(MXS_PIN_TO_GPIO(gpio_info_tmp->pin), 0);
		break;

	case GET_GPIO_VALUE:
		gpio_direction_input(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));
		data = gpio_get_value(MXS_PIN_TO_GPIO(gpio_info_tmp->pin));
		data = data ? 1 : 0;

		copy_to_user((void *)arg, (void *)(&data), sizeof(int));
		break;
	
	default:
		printk("cmd error \n");
		
		return -1;
	}

	return 0;
}

static struct file_operations gpio_fops={
	.owner		= THIS_MODULE,
	.open 		= gpio_open,
	.write		= gpio_write,
	.read		= gpio_read,
	.release	= gpio_release,
	.ioctl		= gpio_ioctl,
};
	
static int __init gpio_init(void)
{
	int i = 0;
	int ret = 0;
	
	for (i = 0; all_gpios_info[i].pin != 0; i++) {
		all_gpios_info[i].pmiscdev = kmalloc(sizeof(struct miscdevice), GFP_KERNEL);
		if (all_gpios_info[i].pmiscdev == NULL) {
			printk("unable to malloc memory \n");
			return -1;
		}
		memset(all_gpios_info[i].pmiscdev, 0, sizeof(struct miscdevice));
		all_gpios_info[i].pmiscdev->name  = all_gpios_info[i].pin_name;
		all_gpios_info[i].pmiscdev->fops  = &gpio_fops;	
		all_gpios_info[i].pmiscdev->minor = MISC_DYNAMIC_MINOR;

		ret = misc_register(all_gpios_info[i].pmiscdev);
		if (ret) {
			printk("misc regist faile \n");
			return -1;
		}

		gpio_info_file[all_gpios_info[i].pmiscdev->minor] = &(all_gpios_info[i]);
		
		printk("build device i:%d dev:/dev/%s \n", i, all_gpios_info[i].pmiscdev->name);
	}

	printk("DTU-M28x gpio driver up. \n"); 

	return 0;
}

static void __exit gpio_exit(void)
{
	int i = 0;

	for (i = 0; all_gpios_info[i].pin != 0; i++) {
		misc_deregister(all_gpios_info[i].pmiscdev);
	}
	printk("DTU-M28x gpio driver down.\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("luozhizhuo, ZhiYuan Electronics Co, Ltd.");
MODULE_DESCRIPTION("GPIO DRIVER FOR iMX287.");


