#include <common.h>
#include <asm/io.h>
#include <asm/arch/ldo.h>
#include <asm/arch/sprd_reg_ahb.h>
#include <asm/arch/regs_ahb.h>
#include <asm/arch/common.h>
#include <asm/arch/adi_hal_internal.h>
#include <asm/u-boot.h>
#include <part.h>
#include <sdhci.h>
#include <asm/arch/mfp.h>
#include <linux/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmap.h>
DECLARE_GLOBAL_DATA_PTR;

extern void sprd_gpio_init(void);
extern void ADI_init (void);
extern int LDO_Init(void);
extern void ADC_Init(void);
extern void init_ldo_sleep_gr(void);

#ifdef CONFIG_GENERIC_MMC
int mv_sdh_init(u32 regbase, u32 max_clk, u32 min_clk, u32 quirks);
int mmc_sdcard_init();

int board_mmc_init(bd_t *bd)
{
	mmc_sdcard_init();

	mv_sdh_init(SDIO0_BASE_ADDR, SDIO_BASE_CLK_192M,
			SDIO_CLK_390K, 0);

	return 0;
}
#endif

extern struct eic_gpio_resource sprd_gpio_resource[];

int board_init()
{
	gd->bd->bi_arch_number = MACH_TYPE_OPENPHONE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	ADI_init();
	misc_init();
	LDO_Init();
	ADC_Init();
	pin_init();
	sprd_eic_init();
	sprd_gpio_init();
	init_ldo_sleep_gr();
	TDPllRefConfig(1);

	return 0;
}

PUBLIC phys_size_t get_dram_size_from_gd(void)
{
	return gd->ram_size;
}

int dram_init(void)
{
#ifdef CONFIG_DDR_AUTO_DETECT
        ulong sdram_base = CONFIG_SYS_SDRAM_BASE;
        ulong sdram_size = 0;
        int i;

        gd->ram_size = 0;
        ulong bank_cnt = CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM;

        for (i = 1; i <= *(volatile uint32 *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM; i++) {
                gd->ram_size += *(volatile ulong *)((volatile ulong *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM + i);
        }

        gd->ram_size = get_ram_size((volatile void *)sdram_base, gd->ram_size);
#else
	gd->ram_size = get_ram_size((volatile void *)PHYS_SDRAM_1,
                                    PHYS_SDRAM_1_SIZE);
#endif
	return 0;
}
