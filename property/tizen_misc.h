#ifndef __TIZEN_MISC_H__
#define __TIZEN_MISC_H__

struct thor_part_info {
	unsigned int offset;
	unsigned int size;
	unsigned long blksz;
	char valid;
	char erase;
};

enum tizen_pm_state {
	PM_STATE_NORMAL = 0,
	PM_STATE_LPM	= 1,
};

#define HDR_BOOT_MAGIC		0x744f6f42	/* BoOt */
#define HDR_SIZE			sizeof(struct sig_header)

/* Size of u-boot-mmc.bin - should be always padded to 1MB */
#define CONFIG_SIG_IMAGE_SIZE	SZ_1M

/* HDR_SIZE - 512 */
struct sig_header {
	unsigned int magic;	/* image magic number */
	unsigned int size;	/* image data size */
	unsigned int valid;	/* valid flag */
	char date[12];		/* image creation timestamp - YYMMDDHH */
	char version[24];	/* image version */
	char bd_name[16];	/* target board name */
	char reserved[448];	/* reserved */
};

int check_board_signature(char *fname, unsigned int dn_addr, unsigned int size);
unsigned int tizen_get_jig_state(void);

enum tizen_pm_state check_pm_status(void);
unsigned int tizen_get_part_num(const char *part_name);
unsigned int tizen_get_part_info(const char *name, struct thor_part_info *info);
#endif
