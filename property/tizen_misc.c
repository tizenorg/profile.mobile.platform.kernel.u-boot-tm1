#include <common.h>
#include <sprd_battery.h>
#include <part.h>
#include <boot_mode.h>
#include <ext2fs.h>
#include <asm/arch/sprd_keypad.h>
#include <asm/arch/sprd_eic.h>
#include "../disk/part_uefi.h"
#include "../drivers/mmc/card_sdio.h"
#include "tizen_misc.h"
#include "tizen_part.h"
#include <normal_mode.h>
#include <asm/arch-sc8830/adi_reg_v3.h>
#include <asm/arch/adi_hal_internal.h>

static void convert_to_string(wchar_t *crap, char *buf)
{
	while (*buf++ = (char)*crap++);
}

int tizen_get_partition_info_by_name (block_dev_desc_t *dev_desc,
							wchar_t* partition_name, disk_partition_t *info)
{
	int i, ret;
	char part_name[72 / sizeof(unsigned short)];

	ret = get_partition_info_by_name(dev_desc, partition_name, info);
	if (ret < 0) {
		convert_to_string(partition_name, part_name);

		for (i = 0; i < ARRAY_SIZE(thor_part_map); i++) {
			if (!strcmp(part_name, thor_part_map[i].part_name)) {
				info->start = thor_part_map[i].start;
				info->size = thor_part_map[i].size;
				info->blksz = thor_part_map[i].blksz;
				return 0;
			}
		}
		return -1;
	}

	return 0;
}

int thor_save_env(char *str)
{
	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;
	int ret;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"param", &part_info);
	if (ret < 0)
		return -1;

#ifndef CONFIG_TIZEN_LPM_SUPPORT
	return ret;
#endif
	ret = Emmc_Write(PARTITION_USER, part_info.start, 1, str);

	return ret;
}

char *thor_get_env(void)
{
	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;
	char str[EMMC_SECTOR_SIZE];
	int ret;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return NULL;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"param", &part_info);
	if (ret < 0)
		return NULL;

	ret = Emmc_Read(PARTITION_USER, part_info.start, 1, str);
	if (ret <0)
		return NULL;

	return str;
}

enum tizen_pm_state check_pm_status(void)
{
	enum tizen_pm_state state = PM_STATE_NORMAL;
	char *str = thor_get_env();

#ifndef CONFIG_TIZEN_LPM_SUPPORT
	return PM_STATE_NORMAL;
#endif

	if (str) {
		if (!strncmp(str, "thor", strlen("thor")))
			return PM_STATE_NORMAL;
	}

	if (charger_connected()) {
		int adp_type = sprdchg_charger_is_adapter();

		switch (adp_type) {
		case ADP_TYPE_CDP:
		case ADP_TYPE_DCP:
		case ADP_TYPE_SDP:
			state = PM_STATE_LPM;
			break;
		}
	}

	return state;
}

unsigned int tizen_get_part_num(const char *part_name)
{
	block_dev_desc_t *p_block_dev;
	int i, ret;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = get_partition_num_by_name(p_block_dev, part_name);
	if (ret < 0) {
		for (i = 0; i < ARRAY_SIZE(thor_part_map); i++) {
			if (!strcmp(part_name, thor_part_map[i].part_name))
				return thor_part_map[i].part_num;
		}
		return -1;
	}

	return ret;
}

unsigned int tizen_get_part_info(const char *name, struct thor_part_info *info)
{
	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;
	wchar_t partition_name[MAX_UTF_PARTITION_NAME_LEN];
	int i, ret;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	for (i = 0; i < MAX_UTF_PARTITION_NAME_LEN; i++) {
		partition_name[i] = name[i];
		if (!name[i])
			break;
	}

	ret = tizen_get_partition_info_by_name(p_block_dev, partition_name, &part_info);
	if (ret < 0) {
		info->valid = 0;
		return -1;
	}

	if (!strcmp(name, PARTS_BOOT))
		info->erase = 1;
	else
		info->erase = 0;

	info->offset = part_info.start;
	info->size = part_info.size;
	info->blksz = part_info.blksz;
	info->valid = 1;

	return ret;
}

unsigned int thor_get_part_info(struct thor_part_info *part_info, const char *name)
{
	int i;
	const char *file_name = strtok(name, ".");

	for (i = 0; i < ARRAY_SIZE(thor_part_map); i++) {
		if (!strcmp(file_name, thor_part_map[i].file_name))
			return tizen_get_part_info(thor_part_map[i].part_name, part_info);
	}

	return -1;
}

void tizen_get_emmc_serial_number(unsigned int *uid) {
	unsigned char *regCID = get_global_cid();
	int v_index = 0;

	for (v_index = 0; v_index < 4; v_index++)
	{
		uid[v_index] = (unsigned int)(regCID[(v_index * 4)]) << 24 |
					(unsigned int)(regCID[(v_index * 4) + 1]) << 16 |
					(unsigned int)(regCID[(v_index * 4) + 2]) << 8 |
					(unsigned int)(regCID[(v_index * 4) + 3]);
	}
}

unsigned int tizen_board_key_scan(void)
{
	unsigned int s_int_status = REG_KPD_INT_RAW_STATUS;
	unsigned int s_key_status = REG_KPD_KEY_STATUS;
	unsigned int scan_code = 0;
	unsigned int key_code = 0;

	if(s_key_status &KPD1_KEY_STS)
		scan_code = (uint32_t)(s_key_status & (KPD1_KEY_STS | KPD1_ROW_CNT | KPD1_COL_CNT));

	/* get volumn_down_status2 */
	if(get_volumn_down_status2())
		scan_code += 0x90;

	if(s_int_status)
		REG_KPD_INT_CLR = KPD_INT_ALL;

	/*
	 *	VOL_UP = 0x80
	 *	VOL_DOWN = 0x122
	 *	HOME = 0x81
	 *	PWR = 0x82
	 *	PWR + VOL_DOWN = 0x112
	 *	PWR + VOLDOWN + HOME = 0x111
	 */
	printf("[tizen] scan code: 0x%x\n", scan_code);
	return scan_code;
}
#define TIZEN_KEY_VOLUP				0x80
#define TIZEN_KEY_VOLDN				0x122
#define TIZEN_KEY_HOME				0x81
#define TIZEN_KEY_PWR				0x82

#define TIZEN_KEY_VOLDN_HOME_PWR	0x111
#define TIZEN_KEY_VOLDN_PWR			0x112

boot_mode_enum_type tizen_check_keypad(void)
{
	unsigned int scan_code;

	scan_code = tizen_board_key_scan();

	switch (scan_code) {
	case TIZEN_KEY_VOLDN_HOME_PWR:
		return CMD_THOR_MODE;
	case TIZEN_KEY_VOLDN_PWR:
		return CMD_FASTBOOT_MODE;
	default:
		return CMD_NORMAL_MODE;
	}

	return CMD_NORMAL_MODE;
}

typedef struct _NV_Update{
    uint32 nv_id;
    void   (*update)();
} NV_Update_T;

typedef enum _NV_UPDATE_ERR_E
{
    ERR_FIXNV_NONE,                    /* normal start */
    ERR_UPDATE_OK,              /* fixnv update successfully, back up some items */
    ERR_FIXNV_INITCPY,          /* fixnv not exist, init copy from dlnv */
    ERR_FIXNV_UPDATENOBACKUP,  /* copy from dlnv to fixnv , nothing backup */
    ERR_FIXNV_ERR_STRUCT,
    ERR_FIXNV_ERR_DATE,
    ERR_FIXNV_ERR_BACKUP_CNT,
    ERR_DLNV_NOTEXIST,          /* maybe DLNV be crashed in AP */
    ERR_FIXNV_NOTEXIST,
    ERR_FIXNV_CRC_FAILURE,
    ERR_FIXNV_UPDATE_FAILURE,
    ERR_FIXNV_UPDATE_CPY,
    ERR_FIXNV_POINTER_ERR,
    ERR_LAST
} NV_UPDATE_ERR_E;

typedef enum _CRC_CHECK_ERR_E
{
	ERROR_NONE,
	ERROR_DATE,
	ERROR_STRUCT,
	ERROR_MAX
}CRC_CHECK_ERR;

typedef struct
{
    uint16 nvitem_id;
    uint16 nv_length;
    uint32 nv_offset;
} _NVITEM_SCAN_T;


int runtimenv_read_with_backup(void)
{
	char *bakbuf = NULL;
	char *oribuf = NULL;
	u8 status = 0;
	char header[SECTOR_SIZE];
	unsigned int checksum = 0;
	nv_header_t * header_p = NULL;
	unsigned int runtimenv_adr;
	int ret;

	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;

	runtimenv_adr = WRUNTIMENV_ADR;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"runtimenv1", &part_info);
	if (ret < 0)
		return -1;

	header_p = (nv_header_t *)header;
	bakbuf = malloc(RUNTIMENV_SIZE + SECTOR_SIZE);

	if(bakbuf)
		memset(bakbuf, 0xff, RUNTIMENV_SIZE + SECTOR_SIZE);
	else
		return -1;

	oribuf = malloc(RUNTIMENV_SIZE + SECTOR_SIZE);

	if(oribuf) {
		memset(oribuf,0xff, RUNTIMENV_SIZE + SECTOR_SIZE);
	} else {
		printf("%s: oribuf malloc failed\n", __func__);
		free(bakbuf);
		return -1;
	}

	//mmc_bread((u8 *)oribuf, base_sector, RUNTIMENV_SIZE);
	ret = Emmc_Read(PARTITION_USER, part_info.start, RUNTIMENV_SIZE / SECTOR_SIZE, oribuf);
	if (ret < 0)
		return -1;

	memset(header, 0, SECTOR_SIZE);
	memcpy(header, oribuf, SECTOR_SIZE);
	checksum = header_p->checksum;

	printf("runtimenv_read_with_backup origin checksum 0x%x\n", checksum);
	if (_chkNVEcc(oribuf + SECTOR_SIZE, RUNTIMENV_SIZE, checksum)) {
		memcpy(runtimenv_adr,oribuf+SECTOR_SIZE,RUNTIMENV_SIZE);
		status += 1;
	}

	ret = tizen_get_partition_info_by_name(p_block_dev, L"runtimenv2", &part_info);
	if (ret < 0)
		return -1;

	//mmc_bread((u8 *)bakbuf, base_sector, RUNTIMENV_SIZE);
	ret = Emmc_Read(PARTITION_USER, part_info.start, RUNTIMENV_SIZE / SECTOR_SIZE, bakbuf);
	if (ret < 0)
		return -1;

	memset(header, 0, SECTOR_SIZE);
	memcpy(header, bakbuf, SECTOR_SIZE);

	checksum = header_p->checksum;
	printf("runtime_read_with_backup backup checksum 0x%x\n", checksum);

	if(_chkNVEcc(bakbuf + SECTOR_SIZE, RUNTIMENV_SIZE, checksum)) {
		memcpy(runtimenv_adr,bakbuf+SECTOR_SIZE,RUNTIMENV_SIZE);
		status += 1 << 1;
	}

	switch(status) {
	case 0:
		printf("both org and bak partition are damaged!\n");
		free(bakbuf);
		free(oribuf);
		return -1;
	case 1:
		printf("bak partition is damaged!\n");
//		mmc_bwrite((u8 *)oribuf, base_sector, (RUNTIMENV_SIZE+SECTOR_SIZE));
		Emmc_Write(PARTITION_USER, part_info.start, (RUNTIMENV_SIZE + SECTOR_SIZE) / SECTOR_SIZE, oribuf);
		break;
	case 2:
		printf("org partition is damaged!\n!");
		memcpy(runtimenv_adr, bakbuf + SECTOR_SIZE, RUNTIMENV_SIZE);

		ret = tizen_get_partition_info_by_name(p_block_dev, L"runtimenv1", &part_info);
		if (ret < 0)
			return -1;

		Emmc_Write(PARTITION_USER, part_info.start, (RUNTIMENV_SIZE + SECTOR_SIZE) / SECTOR_SIZE, bakbuf);
		break;
	case 3:
		printf("both org and bak partition are ok!\n");
			break;
	}

	free(bakbuf);
	free(oribuf);

	return 0;
}

int fixnv_read_with_backup(void)
{
	unsigned int fixnv_adr = WFIXNV_ADR;
	nv_header_t * header_p = NULL;
	nv_header_t *tmp_header_p = NULL;
	unsigned char status = 0;
	char tmp_header[SECTOR_SIZE];
	char header[SECTOR_SIZE];
	char *tmpbuf = NULL;
	char *bakbuf = NULL;
	char *oribuf = NULL;
	int ret;
	unsigned int checksum = 0;
	unsigned int nv_updated = 0;
	unsigned int nv_magic = 0;

	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv2", &part_info);
	if (ret < 0)
		return -1;

	tmp_header_p = (nv_header_t *)tmp_header;
	tmpbuf = malloc(FIXNV_SIZE + SECTOR_SIZE);
	memset(tmp_header, 0, SECTOR_SIZE);

//	mmc_bread((u8 *)tmpbuf, ppi->blkstart, FIXNV_SIZE+SECTOR_SIZE);
	ret = Emmc_Read(PARTITION_USER, part_info.start, (FIXNV_SIZE + SECTOR_SIZE) / SECTOR_SIZE, tmpbuf);
	if (ret < 0)
		return -1;

	memcpy(tmp_header, tmpbuf, SECTOR_SIZE);

	if (tmp_header_p->magic != NV_HEAD_MAGIC) {
		tmp_header_p->magic = NV_HEAD_MAGIC;
		tmp_header_p->len = FIXNV_SIZE;
		tmp_header_p->checksum = (unsigned long)calc_checksum(tmpbuf, FIXNV_SIZE);
		tmp_header_p->version = NV_VERSION;
		tmp_header_p->updated = NV_UPDATED;

		Emmc_Erase(PARTITION_USER, part_info.start, ((FIXNV_SIZE + SECTOR_SIZE) / SECTOR_SIZE));

//		mmc_bwrite(tmp_header,ppi->blkstart,SECTOR_SIZE);
//		mmc_bwrite(tmpbuf,ppi->blkstart+1,FIXNV_SIZE);
		Emmc_Write(PARTITION_USER, part_info.start, 1, tmp_header);
		Emmc_Write(PARTITION_USER, part_info.start + 1, FIXNV_SIZE / SECTOR_SIZE, tmpbuf);
	}

	free(tmpbuf);

	ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv1", &part_info);
	if (ret < 0)
		return -1;

	header_p = (nv_header_t *)header;
	bakbuf = malloc(FIXNV_SIZE + SECTOR_SIZE);

	if (bakbuf)
		memset(bakbuf, 0xff, FIXNV_SIZE + SECTOR_SIZE);
	else
		return -1;

	oribuf = malloc(FIXNV_SIZE + SECTOR_SIZE);
	if(oribuf) {
		memset(oribuf,0xff, FIXNV_SIZE + SECTOR_SIZE);
	} else {
		free(bakbuf);
		return -1;
	}

	printf("loading fixnv1 from %x(%x) to %x\n", part_info.start, FIXNV_SIZE, fixnv_adr);

	//mmc_bread((u8 *)oribuf, base_sector, FIXNV_SIZE);
	ret = Emmc_Read(PARTITION_USER, part_info.start, FIXNV_SIZE / SECTOR_SIZE, oribuf);
	if (ret < 0)
		return -1;

	memset(header, 0, SECTOR_SIZE);
	memcpy(header, oribuf, SECTOR_SIZE);
	checksum = header_p->checksum;

	printf("nv_read_with_backup origin checksum 0x%x\n", checksum);
	if (_chkNVEcc(oribuf + SECTOR_SIZE, FIXNV_SIZE, checksum)) {
		memcpy(fixnv_adr, oribuf + SECTOR_SIZE, FIXNV_SIZE);
		status += 1;
	}
	nv_magic = header_p->magic;

	printf("fixnv1 header magic 0x%x\n",nv_magic);
	if(nv_magic != NV_HEAD_MAGIC)
		status = 0;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv2", &part_info);
	if (ret < 0)
		return -1;

	printf("loading fixnv2 from %x(%x) to %x\n", part_info.start, FIXNV_SIZE, fixnv_adr);
	//mmc_bread((u8 *)bakbuf, base_sector, FIXNV_SIZE);
	ret = Emmc_Read(PARTITION_USER, part_info.start, FIXNV_SIZE / SECTOR_SIZE, bakbuf);
	if (ret < 0)
		return -1;

	memset(header, 0, SECTOR_SIZE);
	memcpy(header, bakbuf, SECTOR_SIZE);

	nv_updated = header_p->updated;
	printf("nv_updated: %x, NV_UPDATED: %x\n", nv_updated, NV_UPDATED);
	printf("nv1_magic: %x, NV_HEAD_MAGIC: %x\n", nv_magic, NV_HEAD_MAGIC);

	if ((nv_updated == NV_UPDATED) && (nv_magic == NV_HEAD_MAGIC)) {
		unsigned int nv_backup_status;
		char *bakbuf_update = NULL;
		char *oribuf_update = NULL;

		printf("updated nvitem !!\n");
		// 1. load fixnv1 to oribuf
		oribuf_update = malloc(FIXNV_SIZE);
		if (!oribuf_update) {
			printf("%s: oribuf_update malloc failed\n", __func__);
			free(bakbuf);
			free(oribuf);
			return -1;
		}

		memset(oribuf_update, 0xff, FIXNV_SIZE);
		memcpy(oribuf_update, oribuf + SECTOR_SIZE, FIXNV_SIZE);

		// 2. load fixnv2 to backbuf
		bakbuf_update = malloc(FIXNV_SIZE);

		if (!bakbuf_update) {
			printf("%s: bakbuf_update malloc failed\n", __func__);
			free(bakbuf);
			free(oribuf);
			free(oribuf_update);
			return -1;
		}

		memset(bakbuf_update, 0xff, FIXNV_SIZE);
		memcpy(bakbuf_update, bakbuf + SECTOR_SIZE, FIXNV_SIZE);
		/* TODO */
//		nv_backup_status = modem_update_fixnv_image(bakbuf_update, oribuf_update);
		printf("nv_backup_status = %d \n", nv_backup_status);

		switch (nv_backup_status) {
		case ERR_FIXNV_NONE:
		case ERR_UPDATE_OK:
		case ERR_FIXNV_INITCPY:
		case ERR_FIXNV_UPDATE_CPY:
			printf("nv update is succeeded.\n");

			header_p = (nv_header_t *)header;
			header_p->magic = NV_HEAD_MAGIC;
			header_p->len = FIXNV_SIZE;
			header_p->checksum = (unsigned long)calc_checksum(oribuf_update,FIXNV_SIZE);
			header_p->version = NV_VERSION;
			header_p->updated = 0x0;

			ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv1", &part_info);
			if (ret < 0)
				return -1;

			//mmc_bwrite(header, ppi->blkstart, SECTOR_SIZE);
			//mmc_bwrite(oribuf_update, (ppi->blkstart+1), FIXNV_SIZE);
			Emmc_Write(PARTITION_USER, part_info.start, 1, header);
			Emmc_Write(PARTITION_USER, part_info.start + 1, FIXNV_SIZE / SECTOR_SIZE, oribuf_update);

			ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv2", &part_info);
			if (ret < 0)
				return -1;

			//mmc_bwrite(header, ppi->blkstart, SECTOR_SIZE);
			//mmc_bwrite(oribuf_update, (ppi->blkstart+1), FIXNV_SIZE);
			Emmc_Write(PARTITION_USER, part_info.start, 1, header);
			Emmc_Write(PARTITION_USER, part_info.start + 1, FIXNV_SIZE / SECTOR_SIZE, oribuf_update);

			memset(fixnv_adr, 0xff, FIXNV_SIZE);
			memcpy(fixnv_adr, oribuf_update, FIXNV_SIZE);
			break;
		default:
			printf("nv update is failed. Original NV will be NV partitions.\n");
			header_p->checksum = 0x0;
			header_p->updated = 0x1;

			ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv2", &part_info);
			if (ret < 0)
				return -1;
			//mmc_bwrite(header, ppi->blkstart, SECTOR_SIZE);
			Emmc_Write(PARTITION_USER, part_info.start, 1, header);
			memcpy(fixnv_adr, oribuf_update, FIXNV_SIZE);
		}

		free(bakbuf_update);
		free(oribuf_update);
	} else {
		checksum = header_p->checksum;
        printf("nv_read_with_backup backup checksum 0x%x\n", checksum);
        if (_chkNVEcc(bakbuf + SECTOR_SIZE, FIXNV_SIZE, checksum)) {
				memcpy(fixnv_adr, bakbuf + SECTOR_SIZE,FIXNV_SIZE);
                status += 1<<1;
        }

        switch(status) {
		case 0:
			printf("both org and bak partition are damaged!\n");
			free(bakbuf);
			free(oribuf);
			return -1;
		case 1:
			printf("bak partition is damaged!\n");
			//mmc_bwrite((u8 *)oribuf, base_sector, (FIXNV_SIZE+SECTOR_SIZE));
			Emmc_Write(PARTITION_USER, part_info.start, (FIXNV_SIZE + SECTOR_SIZE) / SECTOR_SIZE, oribuf);

			break;
		case 2:
			printf("org partition is damaged!\n!");
			memcpy(fixnv_adr, bakbuf+SECTOR_SIZE, FIXNV_SIZE);

			ret = tizen_get_partition_info_by_name(p_block_dev, L"fixnv1", &part_info);
			if (ret < 0)
				return -1;
			//mmc_bwrite((u8 *)bakbuf, base_sector, (FIXNV_SIZE+SECTOR_SIZE));
			Emmc_Write(PARTITION_USER, part_info.start, (FIXNV_SIZE + SECTOR_SIZE) / SECTOR_SIZE, bakbuf);
			break;
		case 3:
			printf("both org and bak partition are ok!\n");
			break;
        }
	}

	free(bakbuf);
    free(oribuf);

	return 0;
}

int load_nvitem(void)
{
	int ret;
	ret = fixnv_read_with_backup();
	if (ret < 0)
		return ret;

	ret = runtimenv_read_with_backup();
	if (ret < 0)
		return ret;

	return 0;
}

int load_modem_data(void)
{
	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;
	int ret = 0;
	u32 left;
	u32 nsct;
	char *sctbuf = NULL;
	unsigned char *buf = WMODEM_ADR;

	if(!buf)
		return -1;

	printf("load modem to addr 0x%08x\n", buf);

	nsct = WMODEM_SIZE / EMMC_SECTOR_SIZE;
	left = WMODEM_SIZE % EMMC_SECTOR_SIZE;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"modem", &part_info);
	if (ret < 0)
		return -1;

	ret = Emmc_Read(PARTITION_USER, part_info.start, nsct, buf);
	if (ret < 0)
		return -1;

	if (left) {
		sctbuf = malloc(EMMC_SECTOR_SIZE);
		if (!sctbuf)
			return -1;

		ret = Emmc_Read(PARTITION_USER, part_info.start + nsct, 1, sctbuf);
		if (ret)
			memcpy(buf + (nsct * EMMC_SECTOR_SIZE), sctbuf, left);
		free(sctbuf);
	}

	printf("partition; modem read success!\n");

	return 0;
}

int load_dsp_data(void)
{
	block_dev_desc_t *p_block_dev;
	disk_partition_t part_info;
	int ret = 0;
	u32 left;
	u32 nsct;
	char *sctbuf = NULL;
	unsigned char *buf = WDSP_ADR;

	if(!buf)
		return -1;

	printf("load dsp to addr 0x%08x\n", buf);

	nsct = WDSP_SIZE / EMMC_SECTOR_SIZE;
	left = WDSP_SIZE % EMMC_SECTOR_SIZE;

	p_block_dev = get_dev("mmc", CONFIG_MMC_DEFAULT_DEV);
	if (!p_block_dev)
		return -1;

	ret = tizen_get_partition_info_by_name(p_block_dev, L"dsp", &part_info);
	if (ret < 0)
		return -1;

	ret = Emmc_Read(PARTITION_USER, part_info.start, nsct, buf);
	if (ret < 0)
		return -1;

	if (left) {
		sctbuf = malloc(EMMC_SECTOR_SIZE);
		if (!sctbuf)
			return -1;

		ret = Emmc_Read(PARTITION_USER, part_info.start + nsct, 1, sctbuf);
		if (ret)
			memcpy(buf + (nsct * EMMC_SECTOR_SIZE), sctbuf, left);
		free(sctbuf);
	}

	printf("partition; dsp read success!\n");

	return 0;
}

int get_image_signature(struct sig_header *hdr, unsigned int base_addr,
						unsigned int size)
{
	memcpy((void *)hdr, (const void *)(base_addr + size - HDR_SIZE),
		HDR_SIZE);

	if (hdr->magic != HDR_BOOT_MAGIC)
		return -EFAULT;

	return 0;
}

static int load_binary_to_addr(char *name, unsigned int addr, unsigned int size)
{
	struct thor_part_info info;
	int ret;

	ret = tizen_get_part_info(name, &info);
	if (ret < 0)
		return ret;

	ret = Emmc_Read(PARTITION_USER, info.offset, size / info.blksz, addr);
	if (ret < 0)
		return ret;

	return 0;
}

int check_board_signature(char *fname, unsigned int dn_addr, unsigned int size)
{
	struct sig_header bh_target;
	struct sig_header bh_addr;
	char bl_buf[CONFIG_SIG_IMAGE_SIZE];
	int ret;

	/* only check u-boot-mmc.bin */
	if (strcmp(fname, "u-boot-mmc"))
		return 0;

	/* can't found signature in target - download continue */
	ret = load_binary_to_addr(PARTS_BOOT, bl_buf, CONFIG_SIG_IMAGE_SIZE);
	if (ret < 0)
		return 0;

	ret = get_image_signature(&bh_target, bl_buf, CONFIG_SIG_IMAGE_SIZE);
	if (ret)
		return 0;

	if (size != CONFIG_SIG_IMAGE_SIZE) {
		printf("Bad file size for: %s.\n", fname);
		printf("Expected: %#x bytes, has: %#x bytes.\n",
		       CONFIG_SIG_IMAGE_SIZE, (unsigned)size);
		return -EINVAL;
	}

	/* can't found signature in address - download stop */
	ret = get_image_signature(&bh_addr, dn_addr, CONFIG_SIG_IMAGE_SIZE);
	if (ret) {
		printf("signature not found.\n");
		return -EFAULT;
	}

	if (strncmp(bh_target.bd_name, bh_addr.bd_name,
		    ARRAY_SIZE(bh_target.bd_name))) {
		printf("Invalid!\n");
		return -EPERM;
	}

	return 0;
}

unsigned int tizen_get_jig_state(void)
{
	int pin_data;

	/* read attached device. */
	pin_data = ANA_REG_GET(ADI_EIC_DATA);
	return pin_data & (1 << 1);
}
