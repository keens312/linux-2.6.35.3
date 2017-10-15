/*
 * file name:zhiyuan_bmp8_logo.c
 * this file read logo from flash
 * author: Yuan Lulu <linux@zlgmcu.com>
 * ZLG(C)2012
 */

#include <linux/linux_logo.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/compatmac.h>

#include <asm/uaccess.h>

/* you can change the following 3 lines with the hearware */
#define IMAGE_MTD_NUM	3			/* the numner of mtd device witch contain logo*/
#define IMAGE_MAX_PIXEL	788000	/* the logo's pixels should be less then 788000 */
								/* 788000 ~= 1024*768     					*/

/* don't edit codes below  the line, or you will regret */
#define IMAGE_HEAD_SIZE	54
#define IMAGE_MAX_SIZE (IMAGE_HEAD_SIZE+IMAGE_MAX_PIXEL+1024)
#define COLOR_MAX_NUM	224		/* linux logo just support 224 colors	       */

static unsigned char logo_flash_clut224_data[IMAGE_MAX_PIXEL] __initdata = {0};
static unsigned char logo_flash_clut224_clut[COLOR_MAX_NUM*3] __initdata = {0};

static struct linux_logo logo_flash_clut224 __initdata = {
    .type		= LINUX_LOGO_CLUT224,
    .width		= 0,
    .height		= 0,
    .clutsize	= 0,
    .clut		= logo_flash_clut224_clut,
    .data		= logo_flash_clut224_data,
};
	
const struct linux_logo * __init_refok zhiyuan_get_bmp8_logo(void)
{	
	struct mtd_info *mtd = NULL;
	unsigned char head[IMAGE_HEAD_SIZE] = {0};
	unsigned char *image = NULL; 
	unsigned char *clut = NULL;
	unsigned char *data = NULL;
	int clutsize = 0;
	
	int rt = 0;
	int size = 0;
	int offset = 0;
	int width = 0;
	int height = 0;
	int count = 0;
	int compress = 0;
	int sizeimage = 0;
	int clrused = 0;
	
	int i = 0;
	int j = 0;
	int fi = 0;
	int li = 0;
	int rdmtd_try_time = 0;
	int try_cnt = 0;

	mtd = get_mtd_device(NULL, IMAGE_MTD_NUM);
	if (IS_ERR(mtd)) { 	
		printk("<2> %s-%d:can not get mtd device\n", __FILE__, __LINE__);
		goto exit;
	}

	/* read a block to find a logo picture */
	rdmtd_try_time = ((uint32_t)mtd->size)/mtd->erasesize;
	for (try_cnt = 0; try_cnt < rdmtd_try_time; try_cnt++) {
		mtd->read(mtd, mtd->erasesize*try_cnt, ARRAY_SIZE(head), &rt, head);
		if (rt != ARRAY_SIZE(head)) 
			continue;
		if (head[0] == 'B' && head[1] == 'M') {
			break;
		}
	}
	if (try_cnt == rdmtd_try_time) {
		/* can't find logo in nand flash */
		printk("<2> %s-%d:can't find logo in mtd%d\n", __FILE__, __LINE__, IMAGE_MTD_NUM);
		goto put;
	}
	printk("<2> %s-%d:find a logo in mtd%d, block%d\n", __FILE__, __LINE__, IMAGE_MTD_NUM, try_cnt);
#if 0
	for (i =0; i < 54; i++) {
		printk(" %.2x", head[i]);
		if ((i+1)%16 == 0)
			printk("\n");
	}
	printk("\n");
#endif

	memcpy((char *)&size, &head[2], 4);
	memcpy((char *)&offset, &head[10], 4);
	memcpy((char *)&width, &head[18], 4);
	memcpy((char *)&height, &head[22], 4);
	memcpy((char *)&count, &head[28], 2);
	memcpy((char *)&compress, &head[30], 4);
	memcpy((char *)&sizeimage, &head[34], 4);
	memcpy((char *)&clrused, &head[46], 4);

	if (size<0 || size > IMAGE_MAX_SIZE) {
		printk("<2> %s-%d:the logo file is too big:%d\n", __FILE__, __LINE__, size);
		goto put;
	}
	if (offset > (IMAGE_HEAD_SIZE + 1024)) {
		printk("<2> %s-%d:the offset is not standard:%d\n", __FILE__, __LINE__, offset);
		goto put;
	}
	if (width <= 0 || height <= 0 || width*height > IMAGE_MAX_PIXEL ) {
		printk("<2> %s-%d:pixel is too much:width-%d, height-%d\n", __FILE__, __LINE__, width, height);
		printk("<2> %s-%d:the logo cintain %d pixels at most\n", __FILE__, __LINE__, IMAGE_MAX_PIXEL);
		goto put;
	}
	if (count !=8) {
		printk("<2> %s-%d:the bitwidth should be equal to 8, but it is %d\n", __FILE__, __LINE__, count);
		goto put;
	}
	if (compress != 0) {
		printk("<2> %s-%d:compress should be 0, but it is %d\n", __FILE__, __LINE__, compress);
		goto put;
	}
	if (clrused > COLOR_MAX_NUM ||  clrused < 0) {
		printk("<2> %s-%d:the number of color used  should be little than %d, but it is %d\n",
				__FILE__, __LINE__, COLOR_MAX_NUM, clrused);
	}
	if (sizeimage == 0) {
		sizeimage = size - offset;
//		printk("<2> %s-%d:the size of image is 0, so set it to (size - offset):%d\n",
//				__FILE__, __LINE__, sizeimage);
	}
	
#if 0
	printk("<2> file szie:%x,offset:%x, width:%x, height:%x,count:%x,compress:%x, imagesize:%x, clrused:%x\n",
			size, offset, width, height, count, compress, sizeimage, clrused);
	printk("<2> file szie:%d,offset:%d, width:%d, height:%d,count:%d,compress:%d, imagesize:%d, clrused:%d\n",
			size, offset, width, height, count, compress, sizeimage, clrused);
#endif

	image = kmalloc(size, GFP_KERNEL);	
	if (!image) {
		printk("<2> %s-%d:malloc for image failed\n", __FILE__, __LINE__);
		goto put;
	}
	mtd->read(mtd, mtd->erasesize*try_cnt, size, &rt, image);
	if (rt != size) {
		printk("<2> %s-%d:read image failed\n", __FILE__, __LINE__);
		goto free_image;
	}

	clut = image + IMAGE_HEAD_SIZE;
	data = image + offset;

	logo_flash_clut224.width = width;
	logo_flash_clut224.height = height;

	/*
	 * in BMP file, the clut's content is BGRX(x is alpha)
	 * but in linux logo,the series is RGB
	 */
	clutsize = 0;
	unsigned char last = 0;
	unsigned int real_width = 0;
	unsigned int logo_index = 0;
	/* in BMP file, the head of ervery line must be 4 multiples */
	if (width%4 == 0) 
		real_width = width;
	else 
		real_width = 4 + (width/4)*4;
//	printk("<2> real_width:0x%x, %d\n", real_width, real_width);

	for (i = 0; i < sizeimage; i++) {

		if ( (i%real_width) >= width)
			continue;
		
		fi = data[(height-1-i/real_width)*real_width + i%real_width]*4; 
		for (j = 0; j < clutsize; j++) {
			li = j*3;
			if (clut[fi+2] == logo_flash_clut224_clut[li] &&
					clut[fi+1] == logo_flash_clut224_clut[li+1] &&
					clut[fi] == logo_flash_clut224_clut[li+2] )
				break;
		}
		if (j < clutsize) {
			logo_flash_clut224_data[logo_index++] = j + 32;
			last = j+32;
#if 0
			if (i < 24 ) {
				printk("<2>i=%d j %d li %d , RGB:%02x %02x %02x\n", i, j, li, logo_flash_clut224_clut[li], 
						logo_flash_clut224_clut[li+1], logo_flash_clut224_clut[li+2] );
			}
#endif
		} else if ( j == clutsize && clutsize < COLOR_MAX_NUM ) {
			li = j*3;
			logo_flash_clut224_clut[li] = clut[fi+2];
			logo_flash_clut224_clut[li+1] = clut[fi+1];
			logo_flash_clut224_clut[li+2] = clut[fi]; 
			logo_flash_clut224_data[logo_index++] = j + 32;
			last = j+32;
			clutsize++;	
		} else {
	//		printk("<2> use the last color:%d\n", i);
			/* if the number of color is more than 224, we must ignor the last ones */	
			logo_flash_clut224_data[logo_index++] = last;
		}
	}

	logo_flash_clut224.clutsize = clutsize;
#if 0
	printk("<2> data:\n");
	for (i = 0; i < 200; i ++) {
		printk("<2> %02x", logo_flash_clut224_data[i]);
		if (i && (i+1)%12 == 0) 
			printk("<2> \n");
	}
	printk("<2> clut:\n");
	for (i = 0; i < 100; i ++) {
		printk("<2> %02x", logo_flash_clut224_clut[i]);
		if (i && (i+1)%12 == 0) 
			printk("<2> \n");
	}
#endif

	kfree(image);
	put_mtd_device(mtd);
	return &logo_flash_clut224;

free_image:
	kfree(image);
put:
	put_mtd_device(mtd);
exit:
	return NULL;
}

EXPORT_SYMBOL_GPL(zhiyuan_get_bmp8_logo);



