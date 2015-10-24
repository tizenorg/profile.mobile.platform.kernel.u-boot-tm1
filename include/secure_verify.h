#ifndef SECURE_VERIFY_H
#define SECURE_VERIFY_H

#include <config.h>
#include <common.h>
#include <asm/arch/secure_boot.h>

#define SEC_HEADER_MAX_SIZE 4096

#define BOOTLOADER_HEADER_OFFSET 0x20
typedef struct {
	uint32 mVersion;	// 1
	uint32 mMagicNum;	// 0xaa55a5a5
	uint32 mCheckSum;	//check sum value for bootloader header
	uint32 mHashLen;	//word length
	uint32 mSectorSize;	// sector size 1-1024
	uint32 mAcyCle;		// 0, 1, 2
	uint32 mBusWidth;	// 0--8 bit, 1--16bit
	uint32 mSpareSize;	// spare part size for one sector
	uint32 mEccMode;	// 0--1bit, 1-- 2bit, 2--4bit, 3--8bit, 4--12bit, 5--16bit, 6--24bit
	uint32 mEccPostion;	// ECC postion at spare part
	uint32 mSectPerPage;	// sector per page
	uint32 mSinfoPos;
	uint32 mSinfoSize;
	uint32 mECCValue[27];
	uint32 mPgPerBlk;
	uint32 mImgPage[5];
} NBLHeader;

int secure_header_parser(uint8_t * header_addr);
uint32_t get_code_offset(uint8_t * header_addr);
int secure_verify(wchar_t * name, uint8_t * header, uint8_t * code);

#endif
