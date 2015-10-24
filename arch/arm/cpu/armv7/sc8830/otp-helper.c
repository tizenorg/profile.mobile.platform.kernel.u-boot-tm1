/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* IMPORTANT:
 *
 * TODO:
 */

#include <common.h>
#include <asm/arch-sc8830/otp_help.h>

#define BLK_WIDTH_OTP_EMEMORY			( 8 ) /* bit counts */
#if defined(CONFIG_SPX35L)
#define BLK_ADC_DETA_ABC_OTP			( 7 ) /* start block for ADC otp delta */
#else
#define BLK_ADC_DETA_ABC_OTP			( 8 ) /* start block for ADC otp delta */
#endif

#define BLK_ADC_DETA                    ( 7 )
#define BASE_ADC_P0				711	//3.6V
#define BASE_ADC_P1				830	//4.2V
#define VOL_P0					3600
#define VOL_P1					4200
#define ADC_DATA_OFFSET			128

u32 efuse_read(int id,int blk_index)
{
	return __ddie_efuse_read(blk_index);
}
int efuse_prog(int id,int blk_index,u32 val)
{
	return __ddie_efuse_prog(blk_index,val);

}
int sci_efuse_calibration_get(unsigned int *p_cal_data)
{
	unsigned int deta;
	unsigned short adc_temp;

#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
	//__adie_efuse_block_dump(); /* dump a-die efuse */

        /* verify to write otp data or not */
        adc_temp = (__adie_efuse_read(0) & (1 << 7));
        if (adc_temp)
                return 0;

        deta = __adie_efuse_read_bits(BLK_ADC_DETA_ABC_OTP * BLK_WIDTH_OTP_EMEMORY, 16);
#elif defined(CONFIG_SPX30G)
        __ddie_efuse_block_dump(); /* dump d-die efuse */

        deta = __ddie_efuse_read(BLK_ADC_DETA);
#else
	#warning "AuxADC CAL DETA need fixing"
#endif

	printf("%s() get efuse block %u, deta: 0x%08x\n", __func__, BLK_ADC_DETA, deta);

	deta &= 0xFFFFFF;

	if ((!deta) || (p_cal_data == NULL)) {
		return 0;
	}

	//adc 3.6V
	adc_temp = ((deta >> 8) & 0x00FF) + BASE_ADC_P0 - ADC_DATA_OFFSET;
	p_cal_data[1] = (VOL_P0) | ((adc_temp << 2) << 16);

	//adc 4.2V
	adc_temp = (deta & 0x00FF) + BASE_ADC_P1 - ADC_DATA_OFFSET;
	p_cal_data[0] = (VOL_P1) | ((adc_temp << 2) << 16);

	return 1;
}
