/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/io.h>
#include <asm/arch/pinmap.h>

static pinmap_t  pinmap[] = {
	{REG_PIN_CTRL0,			0},	//modify it later
	{REG_PIN_CTRL1,			0},	//modify it later
	{REG_PIN_CTRL2,			0},   //ap_uart4-->cp0_uart0, ap_uart2-->cp1_uart1
	{REG_PIN_CTRL3,			0},	//modify it later
        {REG_PIN_CTRL4,                    0},
        {REG_PIN_CTRL5,                    0},
#ifndef CONFIG_FPGA
	{REG_PIN_TRACECLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_TRACECTRL,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_TRACEDAT0,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_TRACEDAT1,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_TRACEDAT2,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_TRACEDAT3,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_TRACEDAT4,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_TRACEDAT5,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_TRACEDAT6,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_TRACEDAT7,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_U0TXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U0RXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U0CTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U0RTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_U1TXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U1RXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U2TXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U2RXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U3TXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U3RXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U3CTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U3RTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	//new start
	{REG_PIN_EXTINT2,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_EXTINT3,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_RFSDA2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSCK2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSEN2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_CP2_RFCTL0,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z}, 
	{REG_PIN_CP2_RFCTL1,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP2_RFCTL2,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_FM_RXIQD0,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_FM_RXIQD1,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN0,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN1,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN2,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN3,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN4,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN5,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WIFI_AGCGAIN6,       BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WBENA,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_WBENB,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPSREAL,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPSIMAG,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPSCLK,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	//new end

	{REG_PIN_RFSDA0,              BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSCK0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSEN0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},

	//new start
	{REG_PIN_RFSDA1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSCK1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_RFSEN1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL0,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL1,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL2,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL3,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL4,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL5,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL6,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CP1_RFCTL7,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CP1_RFCTL8,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CP1_RFCTL9,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CP1_RFCTL10,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL11,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL12,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL13,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL14,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP1_RFCTL15,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL0,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL1,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL2,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL3,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL4,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL5,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CP0_RFCTL6,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CP0_RFCTL7,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_XTLEN,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO6,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO7,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO8,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO9,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_U4TXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U4RXD,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_U4CTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_U4RTS,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	//new end

	{REG_PIN_SCL3,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_SDA3,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_SPI0_CSN,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_SPI0_DO,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_SPI0_DI,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SPI0_CLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},

	//new start 
	{REG_PIN_EXTINT0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_EXTINT1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	//new end

	{REG_PIN_SCL1,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_SDA1,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},

	//new start
	{REG_PIN_GPIO0,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO1,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO2,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_GPIO3,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	//new end

	{REG_PIN_SIMCLK0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SIMDA0,              BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_SIMRST0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SIMCLK1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SIMDA1,              BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_SIMRST1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SIMCLK2,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_OE},
	{REG_PIN_SIMDA2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_OE},
	{REG_PIN_SIMRST2,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},

	//new start
	{REG_PIN_MEMS_MIC_CLK0,       BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MEMS_MIC_DATA0,      BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MEMS_MIC_CLK1,       BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MEMS_MIC_DATA1,      BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	//new end

	{REG_PIN_SD1_CLK,             BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD1_CMD,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD1_D0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD1_D1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD1_D2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD1_D3,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_D3,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_D2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_CMD,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_D0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_D1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_CLK1,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_SD0_CLK0,            BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_PTEST,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_ANA_INT,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_EXT_RST_B,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_CHIP_SLEEP,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_XTL_BUF_EN0,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_XTL_BUF_EN1,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_XTL_BUF_EN2,         BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CLK_32K,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	/* FIXME */
#if 1 /* ken.kuang removel please let it use default value */
	{REG_PIN_AUD_SCLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_AUD_DANGL,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_AUD_DANGR,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_AUD_ADD0,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_AUD_ADSYNC,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_AUD_DAD1,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_AUD_DAD0,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_AUD_DASYNC,          BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
#endif
	{REG_PIN_ADI_D,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_ADI_SYNC,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_ADI_SCLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_LCD_CSN1,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_CSN0,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_RSTN,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_LCD_CD,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_FMARK,           BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_WRN,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_RDN,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D3,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D4,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D5,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D6,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D7,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D8,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D9,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D10,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D11,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D12,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D13,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D14,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D15,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D16,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D17,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D18,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D19,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D20,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D21,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D22,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_LCD_D23,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_SPI2_CSN,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_SPI2_DO,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_SPI2_DI,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_SPI2_CLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_CLK,            BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_EMMC_CMD,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D2,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D3,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D4,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D5,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D6,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_D7,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_EMMC_RST,            BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFWPN,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFRB,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFCLE,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFALE,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFCEN0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFCEN1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFREN,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFWEN,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD0,                BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD1,                BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD2,                BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD3,                BIT_PIN_NULL|BITS_PIN_DS(2)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD4,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD5,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_NFD6,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_NFD7,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_NFD8,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_NFD9,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_NFD10,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_NFD11,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_NFD12,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_NFD13,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_NFD14,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_NFD15,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_CCIRCK0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRCK1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRMCLK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRHS,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRVS,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_CCIRD2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD3,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD4,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD5,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD6,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD7,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD8,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRD9,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRRST,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRPD1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_CCIRPD0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SCL0,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_SDA0,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_KEYOUT0,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_KEYOUT1,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_KEYOUT2,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_KEYOUT3,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_KEYOUT4,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_KEYOUT5,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_KEYOUT6,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_Z},
	{REG_PIN_KEYOUT7,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_KEYIN0,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_KEYIN1,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_KEYIN2,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_KEYIN3,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_IE},
	{REG_PIN_KEYIN4,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_IE},
	{REG_PIN_KEYIN5,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_Z},
	{REG_PIN_KEYIN6,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_KEYIN7,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_GPIO4,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_OE},
	{REG_PIN_GPIO5,               BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_OE},
	{REG_PIN_SCL2,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_SDA2,                BIT_PIN_WPUS|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPU|BIT_PIN_SLP_IE},
	{REG_PIN_CLK_AUX0,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},
	{REG_PIN_IIS0DI,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS0DO,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS0CLK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS0LRCK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS0MCK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS1DI,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS1DO,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS1CLK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS1LRCK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS1MCK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS2DI,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS2DO,              BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS2CLK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS2LRCK,            BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_IIS2MCK,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_IE},
	{REG_PIN_MTDO,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MTDI,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MTCK,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MTMS,                BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
	{REG_PIN_MTRST_N,             BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(0)|BIT_PIN_WPU|BIT_PIN_SLP_WPD|BIT_PIN_SLP_Z},
#endif /* CONFIG_FPGA */
};

int  pin_init(void)
{
	int i;
	for (i = 0; i < sizeof(pinmap)/sizeof(pinmap[0]); i++) {
		__raw_writel(pinmap[i].val, CTL_PIN_BASE + pinmap[i].reg);
	}
	return 0;
}

