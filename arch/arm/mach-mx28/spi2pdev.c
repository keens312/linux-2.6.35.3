#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>

#include <linux/fsl_devices.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>

#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <mach/regs-timrot.h>
#include <mach/regs-lradc.h>
#include <mach/regs-ocotp.h>
#include <mach/device.h>
#include <mach/dma.h>

#include <mach/ddi_bc.h>
#include <mach/pinctrl.h>

#include "regs-digctl.h"
#include "device.h"
#include "mx28evk.h"
#include "mx28_pins.h"

static u64 common_dmamask = DMA_BIT_MASK(32);

//void mxs_nop_release(struct device *dev)
//{
//	/* Nothing */
//}

static struct mxs_spi_platform_data spi2_data = {
	.clk = "ssp.2",
	.slave_mode = 0,
};
static struct resource ssp2_resources[] = {
	{
		.start	= SSP2_PHYS_ADDR,
		.end	= SSP2_PHYS_ADDR + 0x2000 - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MXS_DMA_CHANNEL_AHB_APBH_SSP2,
		.end	= MXS_DMA_CHANNEL_AHB_APBH_SSP2,
		.flags	= IORESOURCE_DMA,
	}, {
		.start	= IRQ_SSP2_DMA,
		.end	= IRQ_SSP2_DMA,
		.flags	= IORESOURCE_IRQ,
	}, {
		.start	= IRQ_SSP2,
		.end	= IRQ_SSP2,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device spi2pdev = {
	.name	= "mxs-spi",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(ssp2_resources),
	.resource 		= ssp2_resources,
	.dev			= {
        .dma_mask               = &common_dmamask,
        .coherent_dma_mask      = DMA_BIT_MASK(32),
        .release 				= mxs_nop_release,
		.platform_data 			= &spi2_data,
        },
};

extern void mx28evk_spi2_pins_init(void);

static int __init mx28_init_spi2(void)
{
	mx28evk_spi2_pins_init();
	
	printk("init spi2 platform_device\n");
	return platform_device_register(&spi2pdev);

}

//static void __exit mx28_exit_spi2(void)
//{
//	printk("remove spi2 platform_device\n");
//	platform_device_unregister(&spi2pdev);
//	
//}

//module_init(mx28_init_spi2);
late_initcall(mx28_init_spi2);
//module_exit(mx28_exit_spi2);

MODULE_AUTHOR("chm <chenhaiman@zlg.cn>");
MODULE_DESCRIPTION("spi2 platform_device");
MODULE_LICENSE("GPL");
