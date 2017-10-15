/*
 * driver/watchodg/sp706s.c
 *
 * Hardware Watchdog Driver
 *
 * Copyright (C) 2010 Guangzhou ZHIYUAN Electronic CO.,LTD.
 * Written by Liu Jingwen <liujingwencn@gmail.com>
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/sp706s.h> 



struct hw_wdt {
	struct timer_list timer;
	unsigned short gpio;
	unsigned long expires;
	spinlock_t io_lock;
};

static void inline wdt_feed(int gpio)
{
    gpio_set_value(gpio, 1);
    gpio_set_value(gpio, 0);
}

static void hw_watchdog_reset(unsigned long data)
{
	struct hw_wdt *wdt;

	if (!data)
		return;
	wdt = (struct hw_wdt *)data;

	spin_lock(&wdt->io_lock);
    wdt_feed(wdt->gpio);
	mod_timer(&wdt->timer, jiffies + wdt->expires);
	spin_unlock(&wdt->io_lock);
}

static int __devinit hw_wdt_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct hw_wdt *wdt;
	struct sp706s_platform_data *pdata;

    pr_info("sp706s hw_wdt_probe called.\n");

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		ret = -ENODEV;
		goto out;
	}

	wdt = (struct hw_wdt *)kzalloc(sizeof(struct hw_wdt), GFP_KERNEL);
	if (!wdt) {
		ret = -ENOMEM;
		goto out;
	}

	wdt->gpio = pdata->gpio;
	if (!wdt->gpio) {
		ret = -ENODEV;
		goto free_mem;
	}

	wdt->expires = pdata->expires;
	if (!wdt->expires)
		wdt->expires = HZ;	/* expire per second by default */

	spin_lock_init(&wdt->io_lock);

	ret = gpio_request(wdt->gpio, "watchdog");
	if (ret < 0) {
		dev_dbg(dev, "Cannot request gpio %d\n", wdt->gpio);
		ret = -EBUSY;
		goto free_mem;
	}
	gpio_direction_output(wdt->gpio, 1);
    wdt_feed(wdt->gpio);
	platform_set_drvdata(pdev, wdt);

	setup_timer(&wdt->timer, hw_watchdog_reset, (unsigned long)wdt);
	mod_timer(&wdt->timer, jiffies + HZ / 2);	/* reset watchdog soon */
    
    pr_info("sp706s watchdog driver registered.\n");
	return 0;

free_mem:
	kfree(wdt);
out:
	return ret;
}

static int __devexit hw_wdt_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct hw_wdt *wdt;

	wdt = platform_get_drvdata(pdev);
	if (!wdt) {
		ret = -ENODEV;
		goto out;
	}

	del_timer(&wdt->timer);

	kfree(wdt);

out:
	return ret;
}

static struct platform_driver platform_wdt_driver = {
	.driver = {
		.name = "sp706s",
		.owner	= THIS_MODULE,
	},
	.probe = hw_wdt_probe,
	.remove = __devexit_p(hw_wdt_remove),
};

static int __init hw_wdt_init(void)
{
    pr_info("\n");

	return platform_driver_register(&platform_wdt_driver);
}

static void __exit hw_wdt_exit(void)
{
	platform_driver_unregister(&platform_wdt_driver);
}

core_initcall(hw_wdt_init);
module_exit(hw_wdt_exit);

MODULE_AUTHOR("Liu Jingwen <linux@zlgmcu.com>");
MODULE_DESCRIPTION("DaVinci Watchdog Driver");

MODULE_LICENSE("GPL");
