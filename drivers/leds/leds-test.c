#define         MYDBG 1 
#if MYDBG
#define mydbg(fmt,...) printk("%s,%d:",__func__,__LINE__),printk( pr_fmt(fmt), ##__VA_ARGS__)
#else
#define mydbg(fmt,...)    (void*)0
#endif


/*
 * Freescale MXS PWM LED driver
 *
 * Author: Drew Benedetti <drewb@embeddedalley.com>
 *
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
 * Copyright 2008 Embedded Alley Solutions, Inc All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>

#include <mach/hardware.h>
#include <mach/system.h>
#include <mach/device.h>
#include <mach/regs-pwm.h>

#include <../arch/arm/mach-mx28/mx28_pins.h>

/*
 * PWM enables are the lowest bits of HW_PWM_CTRL register
 */
#define BM_PWM_CTRL_PWM_ENABLE	((1<<(CONFIG_MXS_PWM_CHANNELS)) - 1)
#define BF_PWM_CTRL_PWM_ENABLE(n) ((1<<(n)) & BM_PWM_CTRL_PWM_ENABLE)

#define BF_PWM_PERIODn_SETTINGS					\
		(BF_PWM_PERIODn_CDIV(5) | /* divide by 64 */ 	\
		BF_PWM_PERIODn_INACTIVE_STATE(3) | /* low */ 	\
		BF_PWM_PERIODn_ACTIVE_STATE(2) | /* high */ 	\
		BF_PWM_PERIODn_PERIOD(LED_FULL)) /* 255 cycles */

static void mxs_led_brightness_set(struct led_classdev *pled, enum led_brightness value);

struct led_classdev led_dev = {
	.name = "led-example",								/* 设备名称为led-example	*/
	.brightness_set = mxs_led_brightness_set,
	.default_trigger = "none",							/* 默认使用none触发器		*/
};

#define LED_GPIO	MXS_PIN_TO_GPIO(PINID_LCD_D23)		/* ERR LED的GPIO			*/
static void mxs_led_brightness_set(struct led_classdev *pled, enum led_brightness value)
{
	gpio_direction_output(LED_GPIO, !value);	
}

static int __init led_init(void)
{
	int ret = 0;
	
	ret = led_classdev_register(NULL, &led_dev);					/* 注册LED设备		*/
	if (ret) {
		printk("register led device faile \n");
		return -1;
	}
	gpio_request(LED_GPIO, "led");							/* 申请GPIO			*/
	return 0;
}


static void __exit led_exit(void)
{
	led_classdev_unregister(&led_dev);						/* 注销LED设备	*/
	gpio_free(LED_GPIO);								/* 释放GPIO		*/

}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("chm <chenhaiman@zlg.cn>");
MODULE_DESCRIPTION("LED Test driver");
MODULE_LICENSE("GPL");
