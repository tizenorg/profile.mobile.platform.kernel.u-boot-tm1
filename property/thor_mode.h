/*
 * thor_mode.h - USB TIZEN THOR - internal gadget definitions
 *
 * Copyright (C) 2015 Samsung Electronics
 * Inha Song  <ideal.song@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _THOR_MODE_H_
#define _THOR_MODE_H_

#include <asm/sizes.h>

#define ENOTSUPP		524	/* Operation is not supported */

#define F_NAME_BUF_SIZE			32
#define THOR_PACKET_SIZE		SZ_1M	/* 1 MiB */
#define THOR_STORE_UNIT_SIZE	SZ_16M	/* 16 MiB */

/* same with fastboot buffer address */
#define CONFIG_THOR_TRANSFER_BUFFER		0x80100000

#define VER_PROTOCOL_MAJOR	4
#define VER_PROTOCOL_MINOR	0

enum rqt {
	RQT_INFO = 200,
	RQT_CMD,
	RQT_DL,
	RQT_UL,
};

enum rqt_data {
	/* RQT_INFO */
	RQT_INFO_VER_PROTOCOL = 1,
	RQT_INIT_VER_HW,
	RQT_INIT_VER_BOOT,
	RQT_INIT_VER_KERNEL,
	RQT_INIT_VER_PLATFORM,
	RQT_INIT_VER_CSC,

	/* RQT_CMD */
	RQT_CMD_REBOOT = 1,
	RQT_CMD_POWEROFF,
	RQT_CMD_EFSCLEAR,

	/* RQT_DL */
	RQT_DL_INIT = 1,
	RQT_DL_FILE_INFO,
	RQT_DL_FILE_START,
	RQT_DL_FILE_END,
	RQT_DL_EXIT,

	/* RQT_UL */
	RQT_UL_INIT = 1,
	RQT_UL_START,
	RQT_UL_END,
	RQT_UL_EXIT,
};

struct rqt_box {				/* total: 256B */
	signed int rqt;				/* request id */
	signed int rqt_data;		/* request data id */
	signed int int_data[14];	/* int data */
	char str_data[5][32];		/* string data */
	char md5[32];				/* md5 checksum */
} __attribute__ ((__packed__));

struct rsp_box {				/* total: 128B */
	signed int rsp;				/* response id (= request id) */
	signed int rsp_data;		/* response data id */
	signed int ack;				/* ack */
	signed int int_data[5];		/* int data */
	char str_data[3][32];		/* string data */
} __attribute__ ((__packed__));

struct data_rsp_box {			/* total: 8B */
	signed int ack;				/* response id (= request id) */
	signed int count;			/* response data id */
} __attribute__ ((__packed__));

enum {
	FILE_TYPE_NORMAL,
	FILE_TYPE_PIT,
};

#endif /* _THOR_MODE_H_ */
