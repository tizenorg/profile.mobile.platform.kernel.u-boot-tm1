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
extern int sound_init(void);

#ifdef CONFIG_GENERIC_MMC
int mv_sdh_init(u32 regbase, u32 max_clk, u32 min_clk, u32 quirks);
int mmc_sdcard_init();

int board_mmc_init(bd_t *bd)
{
#if 0
	mmc_sdcard_init();

	mv_sdh_init(CONFIG_SYS_SD_BASE, SDIO_BASE_CLK_192M,
			SDIO_CLK_250K, 0);
#endif
	return 0;
}
#endif

extern struct eic_gpio_resource sprd_gpio_resource[];

/*enable aon timer2 for udelay functions*/
void aon_26M_timer2_enable()
{
	REG32(REG_AON_APB_APB_EB0) |= BIT_AON_TMR_EB;
}

unsigned int hw_revision = 0;
#define GPIO_HW_REV_COUNT 3
unsigned hw_rev_gpio[GPIO_HW_REV_COUNT] = {166, 169, 170};

int get_hw_rev(void)
{
	int i;

	hw_revision = 0;

	for (i = 0 ; i < GPIO_HW_REV_COUNT ; i++) {
		sprd_gpio_request(NULL, hw_rev_gpio[i]);
		sprd_gpio_direction_input(NULL, hw_rev_gpio[i]);
		hw_revision = (hw_revision << 1) | sprd_gpio_get(NULL, hw_rev_gpio[i]);
		sprd_gpio_free(NULL, hw_rev_gpio[i]);
	}

	return hw_revision;
}

int board_init()
{
	gd->bd->bi_arch_number = 0x7df;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x02000100;
	ADI_init();
	sprd_gpio_init();
	misc_init();

	pin_init();
	sprd_eic_init();

	init_ldo_sleep_gr();

	LDO_Init();
	ADC_Init();
//	sound_init();
	//TDPllRefConfig(1);
//	aon_26M_timer2_enable();
	check_smpl();
	get_hw_rev();

	return 0;
}

void check_smpl(void)
{
#ifdef CONFIG_SMPL_MODE
        if(is_real_battery() == 0){
                sci_adi_write_fast(ANA_REG_GLB_SMPL_CTRL0,0x0,1);
        }
#endif
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((volatile void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);
	return 0;
}

void fdt_fixup_chosen_bootargs_board(char *buf, const char *boot_mode, int calibration_mode)
{
	char *p = buf;
	/**
	 * Because of in u-boot, we can't find FDT chosen remove function
	 * and samsung only uses uart to do calibration,
	 * so in samsung board .dts, we remove the "console=ttyS1,115200n8" in chosen node by default.
	 * so in normal mode, we need to append console
	 */
	if (!calibration_mode) {
		p += sprintf(p, "console=ttyS1,115200n8 no_console_suspend");
	} else {
		p += sprintf(p, "console=null");
	}
}
