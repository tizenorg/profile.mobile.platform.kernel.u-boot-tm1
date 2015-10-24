#include <config.h>
#include <common.h>
#include <linux/types.h>
#include <asm/arch/bits.h>
#include <image.h>
#include <linux/string.h>
#include <android_bootimg.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <android_boot.h>
#include <environment.h>
#include <jffs2/jffs2.h>
#include <boot_mode.h>

#ifdef dprintf
#undef dprintf
#endif
#define dprintf(fmt, args...) printf(fmt, ##args)

void factorytest_mode(void)
{
    printf("%s\n", __func__);
#if BOOT_NATIVE_LINUX
    vlx_nand_boot(RECOVERY_PART, CONFIG_BOOTARGS " androidboot.mode=factorytest", BACKLIGHT_ON);
#else
    vlx_nand_boot(RECOVERY_PART, "androidboot.mode=factorytest", BACKLIGHT_ON);
#endif

}
