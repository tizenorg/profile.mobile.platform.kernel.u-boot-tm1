/******************************************************************************
 ** File Name:    sprdfb_main.h                                            *
 ** Author:                                                           *
 ** DATE:                                                           *
 ** Copyright:    2005 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                            *
 ******************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------*
 ** DATE          NAME            DESCRIPTION                                 *
 **
 ******************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>

#include <asm/arch/sprd_lcd.h>
#include <asm/arch/ldo.h>
#if defined CONFIG_SC8830 || (defined CONFIG_SC9630)
#include <asm/arch/sprd_reg_global.h>
#include <asm/arch/adi_hal_internal.h>
#else
#include <asm/arch/sc8810_reg_global.h>
#endif
#include <asm/arch/regs_global.h>
#include <asm/arch/regs_cpc.h>
#include <asm/arch/sprd_reg.h>

#include "sprdfb.h"

#define MTP_LEN 0x21


void *lcd_base = NULL;		/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

short console_col;
short console_row;



extern int sprdfb_panel_probe(struct sprdfb_device *dev);
extern void sprdfb_panel_remove(struct sprdfb_device *dev);

extern struct display_ctrl sprdfb_dispc_ctrl ;

static struct sprdfb_device s_sprdfb_dev = {0};

static uint32_t lcd_id_to_kernel = 0;
static unsigned char mtp_offset[((MTP_LEN + 6) * 2) + 1] = {0};
static uint8_t elvss_offset = 0;
static unsigned char hbm_g[(15 * 2) + 1] = {0};


#define WHTLED_CTL              ANA_LED_CTL
#define WHTLED_PD_SET           BIT_0
#define WHTLED_PD_RST           BIT_1
#define WHTLED_V_SHIFT          2
#define WHTLED_V_MSK            (0x1F << WHTLED_V_SHIFT)

static void __raw_bits_and(unsigned int v, unsigned int a)
{
	__raw_writel((__raw_readl(a) & v), a);
}

static void __raw_bits_or(unsigned int v, unsigned int a)
{
	__raw_writel((__raw_readl(a) | v), a);
}

static void LCD_SetPwmRatio(unsigned short value)
{
#if defined CONFIG_SC8830 || (defined CONFIG_SC9630)
	// to do
#else
	__raw_bits_or(CLK_PWM0_EN, GR_CLK_EN);
	__raw_bits_or(CLK_PWM0_SEL, GR_CLK_EN);
	__raw_bits_or(PIN_PWM0_MOD_VALUE, CPC_LCD_PWM_REG);
	__raw_writel(LCD_PWM_PRESCALE_VALUE, SPRD_PWM0_PRESCALE);
	__raw_writel(value, SPRD_PWM0_CNT);
	__raw_writel(PWM_REG_MSK_VALUE, SPRD_PWM0_PAT_LOW);
	__raw_writel(PWM_REG_MSK_VALUE, SPRD_PWM0_PAT_HIG);
	__raw_bits_or(LCD_PWM0_EN, SPRD_PWM0_PRESCALE);
#endif
}

void LCD_SetBackLightBrightness( unsigned long  value)
{
	unsigned long duty_mod= 0;
	if(value > LCD_PWM_MOD_VALUE)
		value = LCD_PWM_MOD_VALUE;

/*
	if(value < 0)
		value = 0;
*/

	duty_mod = (value << 8) | LCD_PWM_MOD_VALUE;
	LCD_SetPwmRatio(duty_mod);
}

static uint32 get_adie_chipid(void)
{
       uint32 chip_id;
       chip_id = (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xffff) << 16;
       chip_id |= ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xffff;
       return chip_id;
}

void sprd_white_led_init(void)
{
	__raw_writel(0xc000,0x400388d8);

	__raw_writel(0x0100,0x40038020);
	__raw_writel(0xffff,0x40038024);
	__raw_writel(0xffff,0x4003802c);
	__raw_writel(0xffff,0x40038030);

	__raw_writel(0x0100,0x40038020);

	__raw_writel(0x0081,0x400388d4);
//	__raw_writel(0xff80,0x400388d8);
}

#if (!defined(CONFIG_ARCH_SCX35L))

void (*lcd_panel_cabc_pwm_bl)(int brightness) = NULL;

#ifndef CONFIG_SPX20
void set_backlight(uint32_t value)
{
#if (defined(CONFIG_SP8830GGA) || \
	defined(CONFIG_SP8830GEA) || \
	defined(CONFIG_SP8730SEA) || \
	defined(CONFIG_SP7730GGA) || \
	defined(CONFIG_SP7731GEA) || defined(CONFIG_SP7731GEA_HD) || defined(CONFIG_SP8730SEEA_QHD) || defined(CONFIG_SP8730SEEA_JIG) || defined(CONFIG_SP7731GEA_HD2) || defined(CONFIG_SP7731GEA_HD) || defined(CONFIG_SP7731GEA_HDR) || defined(CONFIG_SP7731GEA_QHD) || defined(CONFIG_SP7731GEA_FWVGA) || defined(CONFIG_SP7731GEAOPENPHONE) || \
	defined(CONFIG_SP7731GEA_LC) || \
	defined(CONFIG_SP7731GGA_LC) || defined(CONFIG_SP7730GGA_LC) || \
	defined(CONFIG_SP7730GGAOPENPHONE) || \
	defined(CONFIG_SP5735C2EA) || \
	defined(CONFIG_SC9620OPENPHONE) || \
	defined(CONFIG_SC9620REFERPHONE) ||defined(CONFIG_SP7720))
	
	/*backlight is driven by PWMD (PWMD=PWM3) */
	__raw_bits_or((0x1 << 0), 0x402d0044);//use ext_26m for clk_pwm3 parent clk
	if(0 == value) {
		__raw_writel(0x0000, 0x40260060);
		printf("sprd backlight power off. brightness = %d (use PWM3 for external backlight control)\n", value);
	} else {
		value = (value & 0xff) >> 2;
		/*enbale pwm3*/
		__raw_bits_or((0x1 << 7), 0x402e0000);
		/*config pwm3*/
		__raw_writel((value << 8) | 0xff, 0x40260064);
		__raw_writel(0xffff, 0x4026006c);
		__raw_writel(0xffff, 0x40260070);
		__raw_writel(0x0100, 0x40260060);
		printf("sprd backlight power on. brightness = %d (use PWM3 for external backlight control)\n", value);
	}
	return;
#endif

#if (defined(CONFIG_SP8830) || defined(CONFIG_SPX15))

int white_led = 0;
#if (defined(CONFIG_SPX15))
	#if (defined(CONFIG_EMMC_BOOT))
		uint32 chip_id;
		chip_id = get_adie_chipid();
		printf("adie chip id: 0x%08X\n", chip_id);
		if(0x2711A000 == chip_id) {
			white_led = 1;
			printf("CONFIG_EMMC_BOOT is set, and the adie chip id is 0x2711A000, therefore, white_led=1\n");
		}
		else {
			white_led = 0;
			printf("CONFIG_EMMC_BOOT is set, but the adie chip id is NOT 0x2711A000, therefore, white_led=0\n");
		}
	#else
		white_led = 0;
		printf("CONFIG_EMMC_BOOT is NOT set, therefore, white_led=0\n");
	#endif
#else
	white_led = 1;
	printf("CONFIG_SP8830 is set and CONFIG_SPX15 is NOT set, therefore, white_led=1\n");
#endif

#if (defined(CONFIG_SPX15))
	if(1 == white_led) {
		/*backlight is driven by whiteled */
		sprd_white_led_init();
		if (value == 0) {
			ANA_REG_SET(0x400388d4,0);
			printf("sprd backlight power off (SPX15 use WHITE_LED backlight control)\n");
		} else {
			__raw_writel(0x0181,0x400388d4);
			__raw_writel(0x0480,0x400388d8);
			printf("sprd backlight power on (SPX15 use WHITE_LED backlight control)\n");
		}
	//============================================both white led and pwm
		{
			/*backlight is driven by PWMC (PWMC=PWM2) */
			__raw_bits_or((0x1 << 0), 0x402d0040);//use ext_26m for clk_pwm2 parent clk
			if(0 == value) {
				__raw_writel(0x0000, 0x40260040);
				printf("sprd backlight power off (SPX15 use PWM2 for external backlight control)\n");
			}
			else {
				value = (value & 0xff) >> 2;
				/*enbale pwm2*/
				__raw_bits_or((0x1 << 6), 0x402e0000);
				/*config pwm2*/
				__raw_writel((value << 8) | 0xff, 0x40260044);
				__raw_writel(0xffff, 0x4026004c);
				__raw_writel(0xffff, 0x40260050);
				__raw_writel(0x0100, 0x40260040);
				printf("sprd backlight power on (SPX15 use PWM2 for external backlight control)\n");
			}
		}
	//============================================both white led and pwm
	}
	else {
		/*backlight is driven by PWMC (PWMC=PWM2) */
		__raw_bits_or((0x1 << 0), 0x402d0040);//use ext_26m for clk_pwm2 parent clk
		if(0 == value) {
			__raw_writel(0x0000, 0x40260040);
			printf("sprd backlight power off (SPX15 use PWM2 for external backlight control)\n");
		}
		else {
			value = (value & 0xff) >> 2;
			/*enbale pwm2*/
			__raw_bits_or((0x1 << 6), 0x402e0000);
			/*config pwm2*/
			__raw_writel((value << 8) | 0xff, 0x40260044);
			__raw_writel(0xffff, 0x4026004c);
			__raw_writel(0xffff, 0x40260050);
			__raw_writel(0x0100, 0x40260040);
			printf("sprd backlight power on (SPX15 use PWM2 for external backlight control)\n");
		}
	}
#else
	/*backlight is driven by whiteled */
	if (value == 0) {
		ANA_REG_SET(0x40038894,0);
		printf("sprd backlight power off (SP8830 use WHITE_LED backlight control)\n");
	} else {
		ANA_REG_SET(0x40038894,(ANA_REG_GET(0x40038894)|(0x3 << 7)));
		printf("sprd backlight power on (SP8830 use WHITE_LED backlight control)\n");
	}
#endif

#elif defined(CONFIG_KANAS_W) || defined(CONFIG_KANAS_TD) || defined(CONFIG_PIKEAYOUNG2DTV) || defined(CONFIG_GRANDPRIME3G_VE) || defined(CONFIG_GRANDPRIME_DTV) || defined(CONFIG_TIZENZ3_3G)
	FB_PRINT("set_backlight\n");
	/* GPIO214 */
	static int is_init = 0;

#if defined(CONFIG_PIKEAYOUNG2DTV)
#define PWM_BACKLIGHT_GPIO 234
#endif

	if (!is_init) {
		sprd_gpio_request(NULL, BACKLIGHT_GPIO);
		sprd_gpio_direction_output(NULL, BACKLIGHT_GPIO, 0);
#if defined(CONFIG_GRANDPRIME3G_VE) || defined(CONFIG_GRANDPRIME_DTV) || defined(CONFIG_TIZENZ3_3G)
		sprd_gpio_request(NULL, LCD_LDO_EN_GPIO);
		sprd_gpio_direction_output(NULL, LCD_LDO_EN_GPIO, 1);
#endif
		#if defined(CONFIG_PIKEAYOUNG2DTV)
		sprd_gpio_request(NULL, PWM_BACKLIGHT_GPIO);
		sprd_gpio_direction_output(NULL, PWM_BACKLIGHT_GPIO, 0);
		#endif		
		is_init = 1;
	}

	if (0 == value) {
		sprd_gpio_set(NULL, BACKLIGHT_GPIO, 0);
		#if defined(CONFIG_PIKEAYOUNG2DTV)
		sprd_gpio_set(NULL, PWM_BACKLIGHT_GPIO, 0);
		#endif
	}
	else {
		sprd_gpio_set(NULL, BACKLIGHT_GPIO, 1);
		#if defined(CONFIG_PIKEAYOUNG2DTV)
		sprd_gpio_set(NULL, PWM_BACKLIGHT_GPIO, 1);
		#endif
	}
#endif

#if defined (CONFIG_POCKET2) || defined (CONFIG_CORSICA_VE) ||defined (CONFIG_VIVALTO) || defined (CONFIG_YOUNG2)
	FB_PRINT("sprdfb: [%s] turn on the backlight\n", __FUNCTION__);

	sprd_gpio_request(NULL, 190);
	sprd_gpio_direction_output(NULL, 190, 0);
	if(0 == value){
		sprd_gpio_set(NULL, 190, 0);
	}else
	{
	    sprd_gpio_set(NULL, 190, 1);
	}
#endif

#if (defined(CONFIG_TSHARKWSAMSUNG) || defined(CONFIG_CORE3) || defined(CONFIG_COREPRIME3G_VE) || defined(CONFIG_TSHARK2J2_3G) || defined(CONFIG_TIZENZ3_3G) || defined(CONFIG_GRANDPRIME_DTV)) || defined(CONFIG_GRANDPRIME3G_VE)
        if (lcd_panel_cabc_pwm_bl) {
                lcd_panel_cabc_pwm_bl(value); /* lcd panel CABC PWM auto control */
        } else {
                /*backlight is driven by PWMC (PWMC=PWM2) */
//                __raw_bits_or((0x1 << 0), 0x402d0040);//use ext_26m for clk_pwm2 parent clk^M
                if(0 == value) {
                        __raw_writel(0x0000, 0x40260040);
                        printf("sprd backlight power off (SPX15 use PWM2 for external backlight control)\n");
                }else {
                        /*enbale pwm2*/
                        __raw_bits_or((0x1 << 6), 0x402e0000);
                        /*config pwm2*/
                        __raw_writel((value << 8) | 0xff, 0x40260044);
                        __raw_writel(0xffff, 0x4026004c);
                        __raw_writel(0xffff, 0x40260050);
                        __raw_writel(0x0100, 0x40260040);
                        printf("sprd backlight power on (SPX15 use PWM2 for external backlight control)\n");
                }
        }
#endif

#if defined (CONFIG_SP8825) || defined (CONFIG_SP8825EA) || defined (CONFIG_SP8825EB) ||defined(CONFIG_GARDA)
	__raw_writel(0x101, 0x4C000138);
	__raw_bits_or((1<<5), 0x4B000008);
	__raw_bits_or((1<<8), 0x4A000384);
	__raw_bits_or((1<<8), 0x4A000388);
	__raw_bits_or((1<<8), 0x4A000380);
#endif


#ifdef CONFIG_SC8810_OPENPHONE
	ANA_REG_AND(WHTLED_CTL, ~(WHTLED_PD_SET | WHTLED_PD_RST));
	ANA_REG_OR(WHTLED_CTL,  WHTLED_PD_RST);
	ANA_REG_MSK_OR (WHTLED_CTL, ( (value << WHTLED_V_SHIFT) &WHTLED_V_MSK), WHTLED_V_MSK);
#elif CONFIG_MACH_CORI
	__raw_bits_or((1<<5),  0x8B000008);
	__raw_bits_or((1<<10), 0x8A000384);
	__raw_bits_or((1<<10), 0x8A000388);
	__raw_bits_or((1<<10), 0x8A000380);
#else
	//if (gpio_request(143, "LCD_BL")) {
	//	FB_PRINT("Failed ro request LCD_BL GPIO_%d \n",
	//		143);
	//	return -ENODEV;
	//}
	//gpio_direction_output(143, 1);
	//gpio_set_value(143, 1);
	//__raw_bits_or((1<<5),  0x8B000008);
	//__raw_bits_or((1<<15), 0x8A000384);
	//__raw_bits_or((1<<15), 0x8A000388);
	//__raw_bits_or((1<<15), 0x8A000380);
#ifndef CONFIG_SP8810EA
	LCD_SetBackLightBrightness(value);
#else
	__raw_writel(0x101, 0x8C0003e0);
	__raw_bits_or((1<<5),  0x8B000008);
	__raw_bits_or((1<<15), 0x8A000384);
	__raw_bits_or((1<<15), 0x8A000388);
	__raw_bits_or((1<<15), 0x8A000380);
#endif

#endif
}
#endif
#endif
void save_lcd_id_to_kernel(uint32_t id)
{
	lcd_id_to_kernel = id;
}

uint32_t load_lcd_id_to_kernel(void)
{
	return lcd_id_to_kernel;
}



static char byte2hex(unsigned char b)
{
	if (b >= 16)
		return '0';
	if (b <= 9)
		return '0' + b;
	if (b >= 10 && b <= 15)
		return 'a' + (b - 10);
/*	return '0'; */
}

static void hex_encoder(unsigned char *in, char *out, int sz)
{
	int i = 0;

	for (i = 0; i < sz; i++) {
		*out = byte2hex((in[i] & 0xF0) >> 4);
		out++;
		*out = byte2hex((in[i] & 0x0F));
		out++;
	}
}

void save_mtp_offset_to_kernel(uint8_t *mtp)
{
	hex_encoder((unsigned char *)mtp, mtp_offset, (MTP_LEN + 6));
	mtp_offset[(MTP_LEN + 6) * 2] = '\0';
}

uint8_t *load_mtp_offset_to_kernel(void)
{
	return mtp_offset;
}

void save_elvss_offset_to_kernel(uint8_t elvss)
{
	elvss_offset = elvss;
}

uint8_t load_elvss_offset_to_kernel(void)
{
	return elvss_offset;
}

void save_hbm_offset_to_kernel(uint8_t *hbm)
{
	hex_encoder((unsigned char *)hbm, hbm_g, 15);
	hbm_g[15 * 2] = '\0';
}

uint8_t *load_hbm_offset_to_kernel(void)
{
	return hbm_g;
}

static int real_refresh(struct sprdfb_device *dev)
{
	int32_t ret;

	FB_PRINT("sprdfb: [%s]\n", __FUNCTION__);

	if(NULL == dev->panel){
		printf("sprdfb: [%s] fail (no panel!)\n", __FUNCTION__);
		return -1;
	}

	ret = dev->ctrl->refresh(dev);
	if (ret) {
		printf("sprdfb: failed to refresh !!!!\n");
		return -1;
	}

	return 0;
}
static int sprdfb_probe(void * lcdbase)
{
	struct sprdfb_device *dev = &s_sprdfb_dev;

	printf("sprdfb:[%s]\n", __FUNCTION__);

	set_backlight(0);

#ifdef CONFIG_MACH_CORI
	LDO_SetVoltLevel(LDO_LDO_SIM3, LDO_VOLT_LEVEL1);
	LDO_TurnOnLDO(LDO_LDO_SIM3);
	LDO_SetVoltLevel(LDO_LDO_VDD28, LDO_VOLT_LEVEL3);
	LDO_TurnOnLDO(LDO_LDO_VDD28);
#endif
#ifdef CONFIG_GARDA
	LDO_SetVoltLevel(LDO_LDO_SIM1, LDO_VOLT_LEVEL2);
	LDO_TurnOnLDO(LDO_LDO_SIM1);
#endif
#ifdef LCD_VDD_2V8_TO_3V0
        //need to set v_lcd_3.0v for lcd. LDO_VDD28_V default is 0xA0 =2.8v, 10mv/step, 0xb4=3.0v
        sci_adi_write(ANA_REG_GLB_LDO_V_CTRL3, 0xB4, 0xFF);        
#endif
/*
	__raw_writel((__raw_readl(0x20900208) | 0xAFE), 0x20900208);
	__raw_writel((__raw_readl(0x20900200) | 0xFFFFFFFF), 0x20900200);
	__raw_writel((__raw_readl(0x20900220) | 0x00500000), 0x20900220);
*/
#ifdef CONFIG_SPX15
#if !defined (CONFIG_POCKET2) && !defined (CONFIG_CORSICA_VE) && !defined (CONFIG_VIVALTO) && !defined (CONFIG_YOUNG2)
	sprd_gpio_request(NULL, 190);
	sprd_gpio_direction_output(NULL, 190, 1);
#endif
#endif

	dev->ctrl = &sprdfb_dispc_ctrl;
	dev->ctrl->early_init(dev);

	if (0 != sprdfb_panel_probe(dev)) {
		sprdfb_panel_remove(dev);
		dev->ctrl->uninit(dev);
		printf("sprdfb: failed to probe\n");
		return -EFAULT;
	}

#ifdef CONFIG_FB_LOW_RES_SIMU
#if (defined LCD_DISPLAY_WIDTH) && (defined LCD_DISPLAY_HEIGHT)
	dev->display_width = LCD_DISPLAY_WIDTH;
	dev->display_height = LCD_DISPLAY_HEIGHT;
#else
	dev->display_width = dev->panel->width;
	dev->display_height = dev->panel->height;
#endif
#endif

	dev->smem_start = ((uint32_t)lcdbase);
	dev->ctrl->init(dev);
	return 0;
}


void lcd_initcolregs(void)
{
	FB_PRINT("sprdfb:[%s]\n", __FUNCTION__);
}

void lcd_disable(void)
{
	printf("sprdfb:[%s]\n", __FUNCTION__);
	sprdfb_panel_remove(&s_sprdfb_dev);
	s_sprdfb_dev.ctrl->uninit(&s_sprdfb_dev);
}


/* References in this function refer to respective Linux kernel sources */
void lcd_enable(void)
{
	FB_PRINT("sprdfb:[%s]\n", __FUNCTION__);
}

void lcd_ctrl_init(void *lcdbase)
{
	FB_PRINT("sprdfb:[%s]\n", __FUNCTION__);
	sprdfb_probe(lcdbase);
}

void lcd_display(void)
{
	printf("sprdfb:[%s]\n", __FUNCTION__);
	real_refresh(&s_sprdfb_dev);
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
extern nand_info_t nand_info[];

void lcd_show_board_info(void)
{
    ulong dram_size, nand_size;
    int i;
    char temp[32];

    dram_size = 0;
    for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
      dram_size += gd->bd->bi_dram[i].size;
    nand_size = 0;
    for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
      nand_size += nand_info[i].size;

    lcd_printf("\n%s\n", U_BOOT_VERSION);
    lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
                dram_size >> 20,
                nand_size >> 20 );
    lcd_printf("  Board            : esd ARM9 \n");
    lcd_printf("  Mach-type        : %lu\n", gd->bd->bi_arch_number);
}
#endif /* CONFIG_LCD_INFO */

