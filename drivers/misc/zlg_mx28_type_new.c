
/* 
 *  * XUJI's concentrator LED driver
 *   * Author: Liu Yang <liuyang@edmail.zlgmcu.com>
 *    */
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define	DEV_NAME	"zlg-systemType"

#define  EPC_28xA   0
#define  EPC_283B   1
#define  EPC_287B   2
#define  EPC_280    3
#define  IoT_3960   4
#define  IoT_3962   5
#define  A287_WB128LI  6
#define  A287_W128LI   7

int zlg_board_type = 1;
EXPORT_SYMBOL(zlg_board_type);

static ssize_t zlg_systemType_sysfs_read_attr_name(struct device *dev, struct device_attribute   *attr, char *buf) 	
{											
	switch(zlg_board_type){
		case EPC_28xA:
			return sprintf(buf, "%s\n","EPC_28xA");
			break;
		case EPC_283B:
			return sprintf(buf, "%s\n","EPC_283B");
			break;
		case EPC_287B:
			return sprintf(buf, "%s\n","EPC_287B");
			break;
		case EPC_280:
			return sprintf(buf, "%s\n","EPC_280");
			break;
		case IoT_3960:
			return sprintf(buf, "%s\n","IoT_3960");
			break;
		case IoT_3962:
			return sprintf(buf, "%s\n","IoT_3962");
			break;
		case A287_WB128LI:
			return sprintf(buf, "%s\n","A287_WB128LI");
			break;
		case A287_W128LI:
			return sprintf(buf, "%s\n","A287_W128LI");
			break;
		default:
			return 0;
	}		
}											
static DEVICE_ATTR(board_name, S_IRUGO | S_IWUSR, 					
		   zlg_systemType_sysfs_read_attr_name, NULL);			


static int zlg_systemType_adc_sysfs_register(struct device   *dev)
{
	int	err;

	err	= device_create_file(dev, &dev_attr_board_name);

	return err;
}

static int
zlg_systemType_adc_sysfs_unregister(struct device *dev)
{
	device_remove_file(dev, &dev_attr_board_name);

	return 0;
}

static int __devinit zlg_systemType_adc_probe(struct platform_device   *pdev)
{

	return zlg_systemType_adc_sysfs_register(&pdev->dev);
}

static int __devexit zlg_systemType_adc_remove(struct platform_device  *pdev)
{
	return zlg_systemType_adc_sysfs_unregister(&pdev->dev);
}

static void zlg_systemType_adc_device_release(struct device *dev)
{
	/* 
	 **	register/unregister device in driver is NOT a regular way.
 	 **	So, this function is needed although it's empty.
	 **/
}

static struct platform_device	zlg_systemType_adc_device	= {
	.name	= DEV_NAME,
	.id		= -1,
	.num_resources	= 0,
	.dev	= {
		.release	= zlg_systemType_adc_device_release,
	},
};

static struct platform_driver	zlg_systemType_adc_driver	= {
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
	.probe	= zlg_systemType_adc_probe,
	.remove	= zlg_systemType_adc_remove,
};

static int __init zlg_systemType_adc_init(void)
{
	int	err;

#ifdef CONFIG_EPC_28xA
	zlg_board_type = EPC_28xA;
#endif
#ifdef CONFIG_EPC_283B
	zlg_board_type = EPC_283B;
#endif
#ifdef CONFIG_EPC_287B
	zlg_board_type = EPC_287B;
#endif
#ifdef CONFIG_EPC_280
	zlg_board_type = EPC_280;
#endif
#ifdef CONFIG_IoT_3960
	zlg_board_type = IoT_3960;
#endif
#ifdef CONFIG_IoT_3962
	zlg_board_type = IoT_3962;
#endif
#ifdef CONFIG_A287_WB128LI
	zlg_board_type = A287_WB128LI;
#endif
#ifdef CONFIG_A287_W128LI
	zlg_board_type = A287_W128LI;
#endif

	err	= platform_device_register(&zlg_systemType_adc_device);
	if (err)
		return err;

	return platform_driver_register(&zlg_systemType_adc_driver);
}

static void __exit zlg_systemType_adc_exit(void)
{
	platform_driver_unregister(&zlg_systemType_adc_driver);
	platform_device_unregister(&zlg_systemType_adc_device);
}

module_init(zlg_systemType_adc_init);
module_exit(zlg_systemType_adc_exit);

MODULE_AUTHOR("zhuguojun <zhuguojun@zlg.cn>");
MODULE_DESCRIPTION("driver for M28x device");
MODULE_LICENSE("GPL");

