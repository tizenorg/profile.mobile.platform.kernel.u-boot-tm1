#ifndef _TIZEN_PART_H_
#define _TIZEN_PART_H_

#define F_PART_BUF_SIZE			32

struct part_mapping_table {
	int part_num;
	char part_name[72 / sizeof(unsigned short)];
	char file_name[F_PART_BUF_SIZE];
	unsigned long start;
	unsigned long size;
	unsigned long blksz;
};

struct part_mapping_table thor_part_map[] = {
	{ -1, PARTS_BOOT, "u-boot", 0x11000, 0x1000, 0x200 },
	{ -1, PARTS_BOOT, "u-boot-mmc", 0x11000, 0x1000, 0x200 },
	{ -1, PARTS_KERNEL, "dzImage", 0x16000, 0x4000, 0x200 },
	{ -1, PARTS_RECOVERY, "dzImage-recovery", 0x1a000, 0x4000, 0x200 },
	{ -1, PARTS_MODULE, "modules", 0x26000, 0x6000, 0x200 },
	{ -1, PARTS_MODEM, "SPRDCP", 0x2c000, 0x4000, 0x200 },
	{ -1, PARTS_DSP, "SPRDDSP", 0x30000, 0x2000, 0x200 },
	{ -1, PARTS_FIXNV1, "nvitem-back", 0x2800, 0x800, 0x200 },
	{ -1, PARTS_FIXNV2, "nvitem", 0x3000, 0x800, 0x200 },
	{ -1, PARTS_SYSDATA, "system-data", 0x43000, 0x40000, 0x200 },
	{ -1, PARTS_USER, "user", 0x83000, 0xc1a000, 0x200 },
	{ 26, PARTS_ROOTFS, "rootfs", 0xc9d000, 0x1c0000, 0x200 },
	{ -1, PARTS_RUNTIMENV1, "", 0x5800, 0x800, 0x200 },
	{ -1, PARTS_RUNTIMENV2, "", 0x6000, 0x800, 0x200 },
};

#endif /* _TIZEN_PART_H_ */
