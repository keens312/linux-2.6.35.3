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

//void mxs_nop_release(struct device *dev)
//{
//	/* Nothing */
//}

#if defined(CONFIG_BCMDHD_WEXT) || defined(CONFIG_BCMDHD_WEXT_MODULE)
static u64 common_dmamask = DMA_BIT_MASK(32);

static int mxs_mmc_get_wp_ssp2(void)
{
    return 0;
}

static int mxs_mmc_hw_init_ssp2(void)
{
    //int ret = 0;
    //return ret;
    return 0;
}

static void mxs_mmc_hw_release_ssp2(void)
{

}

static void mxs_mmc_cmd_pullup_ssp2(int enable)
{
    mxs_set_pullup(PINID_SSP0_DATA6, enable, "mmc2_cmd");
}

static unsigned long mxs_mmc_setclock_ssp2(unsigned long hz)
{
    struct clk *ssp = clk_get(NULL, "ssp.2"), *parent;

    if (hz > 1000000)
            parent = clk_get(NULL, "ref_io.1");
    else
            parent = clk_get(NULL, "xtal.0");

    clk_set_parent(ssp, parent);
    clk_set_rate(ssp, 2 * hz);
    clk_put(parent);
    clk_put(ssp);

    return hz;
}
#endif  //end CONFIG_BCMDHD_WEXT

#if defined(CONFIG_BCMDHD_WEXT) || defined(CONFIG_BCMDHD_WEXT_MODULE)
static struct mxs_mmc_platform_data mmc2_data = {
    .hw_init        = mxs_mmc_hw_init_ssp2,
    .hw_release     = mxs_mmc_hw_release_ssp2,
    .get_wp         = mxs_mmc_get_wp_ssp2,
    .cmd_pullup     = mxs_mmc_cmd_pullup_ssp2,
    .setclock       = mxs_mmc_setclock_ssp2,
    .caps           = MMC_CAP_4_BIT_DATA | MMC_CAP_DATA_DDR,
    .min_clk        = 400000,
    .max_clk        = 48000000,
    .read_uA        = 50000,
    .write_uA       = 70000,
    .clock_mmc = "ssp.2",
    .power_mmc = NULL,
};

static struct resource mmc2_resource[] = {
    {
        .flags  = IORESOURCE_MEM,
        .start  = SSP2_PHYS_ADDR,
        .end    = SSP2_PHYS_ADDR + 0x2000 - 1,
    },
    {
        .flags  = IORESOURCE_DMA,
        .start  = MXS_DMA_CHANNEL_AHB_APBH_SSP2,
        .end    = MXS_DMA_CHANNEL_AHB_APBH_SSP2,
    },
    {
        .flags  = IORESOURCE_IRQ,
        .start  = IRQ_SSP2_DMA,
        .end    = IRQ_SSP2_DMA,
    },
    {
        .flags  = IORESOURCE_IRQ,
        .start  = IRQ_SSP2,
        .end    = IRQ_SSP2,
    },
};

static struct platform_device	sdio2pdev	= {
	.name	= "mxs-mmc",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(mmc2_resource),
	.resource 		= mmc2_resource,
	.dev			= {
        .dma_mask               = &common_dmamask,
        .coherent_dma_mask      = DMA_BIT_MASK(32),
        .release 				= mxs_nop_release,
		.platform_data 			= &mmc2_data,
        },
};
#endif  //end CONFIG_BCMDHD_WEXT

extern void mx28evk_sdio2_pins_init(void);

static int __init mx28_init_mmc2(void)
{
#if defined(CONFIG_BCMDHD_WEXT) || defined(CONFIG_BCMDHD_WEXT_MODULE)
	mx28evk_sdio2_pins_init();
	printk("init sdio2 platform_device\n");
	return platform_device_register(&sdio2pdev);
#endif
}

//static void __exit mx28_exit_mmc2(void)
//{
//#if  defined(CONFIG_BCMDHD_WEXT) || defined(CONFIG_BCMDHD_WEXT_MODULE)
//	printk("remove sdio2 platform_device\n");
//	platform_device_unregister(&sdio2pdev);
//#endif
//	
//}

//module_init(mx28_init_mmc2);
late_initcall(mx28_init_mmc2);
//module_exit(mx28_exit_mmc2);

MODULE_AUTHOR("chm <chenhaiman@zlg.cn>");
MODULE_DESCRIPTION("sdio2 platform_device");
MODULE_LICENSE("GPL");
