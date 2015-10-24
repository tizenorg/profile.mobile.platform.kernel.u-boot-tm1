/* drivers/video/sprdfb/lcd_s6e88oa_mipi.c
 *
 * Support for s6e88oa mipi LCD device
 *
 * Copyright (C) 2010 Spreadtrum
 */

#include <asm/arch/sprd_lcd.h>
#include "../sprdfb.h"
#define printk printf

#define  LCD_DEBUG
#ifdef LCD_DEBUG
#define LCD_PRINT printk
#else
#define LCD_PRINT(...)
#endif

#define MAX_DATA   150


typedef struct LCM_Init_Code_tag {
	unsigned int tag;
	unsigned char data[MAX_DATA];
}LCM_Init_Code;

typedef struct LCM_force_cmd_code_tag{
	unsigned int datatype;
	LCM_Init_Code real_cmd_code;
}LCM_Force_Cmd_Code;

#define LCM_TAG_SHIFT 24
#define LCM_TAG_MASK  ((1 << 24) -1)
#define LCM_SEND(len) ((1 << LCM_TAG_SHIFT)| len)
#define LCM_SLEEP(ms) ((2 << LCM_TAG_SHIFT)| ms)
//#define ARRAY_SIZE(array) ( sizeof(array) / sizeof(array[0]))

#define LCM_TAG_SEND  (1<< 0)
#define LCM_TAG_SLEEP (1 << 1)


static LCM_Init_Code init_data[] = {
	{LCM_SLEEP(5)},	/*>5ms*/
	/*level2_comma_set */
	{LCM_SEND(5), {3, 0, 0xF0,\
					0x5A , 0x5A} },
/*	{LCM_SEND(5), {3, 0, 0xFC,\
				0xA5 , 0xA5} },*/

	/*exit_sleep*/
	{LCM_SEND(1), {0x11} },
	{LCM_SLEEP(25)},	/*>120ms*/

	/*avdd set*/
	{LCM_SEND(6), {4, 0, 0xB8,\
				0x38, 0x0B, 0x2D} },


// set brightness

	/*gamma set 350cd*/
	{LCM_SEND(36), {34, 0, 0xCA,\
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x80, 0x80, 0x80,\
		0x00, 0x00, 0x00} },
	/*aid set*/
	{LCM_SEND(8), {6, 0, 0xB2,\
				0x40, 0x08, 0x20, 0x00, 0x08} },
	/*elvss set*/
	{LCM_SEND(5), {3, 0, 0xB6,\
					0x28 , 0x0B} },
	/*acl set*/
	{LCM_SEND(2), {0x55,\
					0x00} },

	/*gamma update*/
	{LCM_SEND(2), {0xF7,\
					0x03} },
	/*display on*/
	{LCM_SEND(1), {0x29} },
	{LCM_SLEEP(120)},	/*>120ms*/

};

static LCM_Init_Code disp_on =  {LCM_SEND(1), {0x29}};

static LCM_Init_Code sleep_in[] =  {
	{LCM_SEND(1), {0x28}},
	{LCM_SLEEP(150)}, 	//>150ms
	{LCM_SEND(1), {0x10}},
	{LCM_SLEEP(150)},	//>150ms
};

static LCM_Init_Code sleep_out[] =  {
	{LCM_SEND(1), {0x11}},
	{LCM_SLEEP(120)},//>120ms
	{LCM_SEND(1), {0x29}},
	{LCM_SLEEP(20)}, //>20ms
};

//extern void (*lcd_panel_cabc_pwm_bl)(int brightness);
//extern void backlight_control(int brigtness);
static int32_t s6e88oa_mipi_init(struct panel_spec *self)
{
	int32_t i;
	LCM_Init_Code *init = init_data;
	unsigned int tag;

	//lcd_panel_cabc_pwm_bl = backlight_control;

	mipi_set_cmd_mode_t mipi_set_cmd_mode = self->info.mipi->ops->mipi_set_cmd_mode;
	mipi_dcs_write_t mipi_dcs_write = self->info.mipi->ops->mipi_dcs_write;
	mipi_eotp_set_t mipi_eotp_set = self->info.mipi->ops->mipi_eotp_set;

	LCD_PRINT("lcd_s6e88oa_init\n");

	mipi_set_cmd_mode();
	mipi_eotp_set(0,0);

	for(i = 0; i < ARRAY_SIZE(init_data); i++){
		tag = (init->tag >>24);
		if(tag & LCM_TAG_SEND){
			mipi_dcs_write(init->data, (init->tag & LCM_TAG_MASK));
			udelay(20);
		}else if(tag & LCM_TAG_SLEEP){
			mdelay(init->tag & LCM_TAG_MASK);//udelay((init->tag & LCM_TAG_MASK) * 1000);
		}
		init++;
	}
	mipi_eotp_set(0,0);

	return 0;
}

static uint32_t s6e88oa_readid(struct panel_spec *self)
{
#if 0
	uint32 j =0;
	uint8_t read_data[3] = {0};
	int32_t read_rtn = 0;
	uint8_t param[2] = {0};
	mipi_set_cmd_mode_t mipi_set_cmd_mode = self->info.mipi->ops->mipi_set_cmd_mode;
	mipi_force_write_t mipi_force_write = self->info.mipi->ops->mipi_force_write;
	mipi_force_read_t mipi_force_read = self->info.mipi->ops->mipi_force_read;
	mipi_eotp_set_t mipi_eotp_set = self->info.mipi->ops->mipi_eotp_set;

	LCD_PRINT("lcd_s6e88oa_mipi read id!\n");

	mipi_set_cmd_mode();
	mipi_eotp_set(0,0);

	for(j = 0; j < 4; j++){
		param[0] = 0x01;
		param[1] = 0x00;
		mipi_force_write(0x37, param, 2);
		read_rtn = mipi_force_read(0xda,1,&read_data[0]);
		read_rtn = mipi_force_read(0xdb,1,&read_data[1]);
		read_rtn = mipi_force_read(0xdc,1,&read_data[2]);
		if((0x52 == read_data[0])&&(0x00 == read_data[1])&&(0x00 == read_data[2]))
		{
			LCD_PRINT("lcd_s6e88oa_mipi read id success!\n");
			return 0x55b8f0;
		}
	}
	LCD_PRINT("lcd_s6e88oa_mipi read id fail! 0xda,0xdb,0xdc is 0x%x,0x%x,0x%x!\n",read_data[0],read_data[1],read_data[2]);
	mipi_eotp_set(0,0);
	return 0;
#endif
	return 0x6e880a;
}

static struct panel_operations lcd_s6e88oa_mipi_operations = {
	.panel_init = s6e88oa_mipi_init,
	.panel_readid = s6e88oa_readid,
};

static struct timing_rgb lcd_s6e88oa_mipi_timing = {
	.hfp = 110,  /* unit: pixel */
	.hbp = 110,
	.hsync = 8,
	.vfp = 13, /*unit: line*/
	.vbp = 2,
	.vsync = 1,
};


static struct info_mipi lcd_s6e88oa_mipi_info = {
	.work_mode  = SPRDFB_MIPI_MODE_VIDEO,
	.video_bus_width = 24, /*18,16*/
	.lan_number = 	2,
	.phy_feq = 450 * 1000,
	.h_sync_pol = SPRDFB_POLARITY_POS,
	.v_sync_pol = SPRDFB_POLARITY_POS,
	.de_pol = SPRDFB_POLARITY_POS,
	.te_pol = SPRDFB_POLARITY_POS,
	.color_mode_pol = SPRDFB_POLARITY_NEG,
	.shut_down_pol = SPRDFB_POLARITY_NEG,
	.timing = &lcd_s6e88oa_mipi_timing,
	.ops = NULL,
};

struct panel_spec lcd_s6e88oa_mipi_spec = {
	.width = 540,
	.height = 960,
	.fps = 60,
	.type = LCD_MODE_DSI,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.mipi = &lcd_s6e88oa_mipi_info
	},
	.ops = &lcd_s6e88oa_mipi_operations,
};
