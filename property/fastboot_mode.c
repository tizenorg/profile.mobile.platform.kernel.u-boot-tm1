#include <config.h>
#include <linux/types.h>
#include <asm/arch/bits.h>
#include <boot_mode.h>
#include <common.h>
#include <linux/string.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <android_boot.h>
#include <environment.h>
#include <jffs2/jffs2.h>
#include "normal_mode.h"

extern int dwc_otg_driver_init(void);
extern int usb_fastboot_initialize(void);


void fastboot_mode(void)
{
	debugf("%s\n", __FUNCTION__);
#ifdef CONFIG_SPLASH_SCREEN
	vibrator_hw_init();
	set_vibrator(1);
	//read boot image header
	size_t size = 1<<19;

	extern void *lcd_base;
	extern int lcd_display_bitmap(ulong bmp_image, int x, int y);
	extern lcd_display(void);
	extern void set_backlight(uint32_t value);
	extern void Dcache_CleanRegion(unsigned int addr, unsigned int length);

	lcd_printf("fastboot mode\n");
#if defined(CONFIG_SC8810) || defined(CONFIG_SC8825) || defined(CONFIG_SC8830) || defined(CONFIG_SC9630)
		Dcache_CleanRegion((unsigned int)(lcd_base), size<<1);//Size is to large.
#endif
	lcd_display();
	set_backlight(255);
	set_vibrator(0);
#endif
        MMU_DisableIDCM();

#ifdef CONFIG_CMD_FASTBOOT
	dwc_otg_driver_init();
	usb_fastboot_initialize();
#endif

	return;
}
