#include <config.h>
#include "normal_mode.h"
#include "../disk/part_uefi.h"
#include "../disk/part_efi.h"
#include "../drivers/mmc/card_sdio.h"
#include "asm/arch/sci_types.h"
#include <ext_common.h>
#include <ext4fs.h>
#ifdef CONFIG_SECURE_BOOT
#include <asm/arch/secure_boot.h>
#include "secure_verify.h"
#endif
#include <asm/arch/sprd_reg.h>
#ifdef CONFIG_ARCH_SCX35L	//only for sharkL branch modem boot process
#include <asm/arch/cp_boot.h>
#endif

#include "dev_tree.h"

#define KERNL_PAGE_SIZE 2048

long long load_image_time = 0;

#ifdef CONFIG_SUPPORT_TDLTE
static boot_image_required_t const s_boot_image_tl_table[] = {
	{L"tl_fixnv1", L"tl_fixnv2", LTE_FIXNV_SIZE, LTE_FIXNV_ADDR},
	{L"tl_runtimenv1", L"tl_runtimenv2", LTE_RUNNV_SIZE, LTE_RUNNV_ADDR},
	{L"tl_modem", NULL, LTE_MODEM_SIZE, LTE_MODEM_ADDR},
	{L"tl_ldsp", NULL, LTE_LDSP_SIZE, LTE_LDSP_ADDR},	//ltedsp
	{L"tl_tgdsp", NULL, LTE_GDSP_SIZE, LTE_GDSP_ADDR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_WLTE
static boot_image_required_t const s_boot_image_wl_table[] = {
	{L"wl_fixnv1", L"wl_fixnv2", LTE_FIXNV_SIZE, LTE_FIXNV_ADDR},
	{L"wl_runtimenv1", L"wl_runtimenv2", LTE_RUNNV_SIZE, LTE_RUNNV_ADDR},
	{L"wl_modem", NULL, LTE_MODEM_SIZE, LTE_MODEM_ADDR},
	{L"wl_ldsp", NULL, LTE_LDSP_SIZE, LTE_LDSP_ADDR},
	{L"wl_gdsp", NULL, LTE_GDSP_SIZE, LTE_GDSP_ADDR},
	{L"wl_warm", NULL, WL_WARM_SIZE, WL_WARM_ADDR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_LTE
static boot_image_required_t const s_boot_image_lte_table[] = {
	{L"l_fixnv1", L"l_fixnv2", LTE_FIXNV_SIZE, LTE_FIXNV_ADDR},
	{L"l_runtimenv1", L"l_runtimenv2", LTE_RUNNV_SIZE, LTE_RUNNV_ADDR},
	{L"l_modem", NULL, LTE_MODEM_SIZE, LTE_MODEM_ADDR},
	{L"l_ldsp", NULL, LTE_LDSP_SIZE, LTE_LDSP_ADDR},
	{L"l_gdsp", NULL, LTE_GDSP_SIZE, LTE_GDSP_ADDR},
	{L"l_warm", NULL, WL_WARM_SIZE, WL_WARM_ADDR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_TD
static boot_image_required_t const s_boot_image_TD_table[] = {
	{L"tdfixnv1", L"tdfixnv2", FIXNV_SIZE, TDFIXNV_ADR},
	{L"tdruntimenv1", L"tdruntimenv2", RUNTIMENV_SIZE, TDRUNTIMENV_ADR},
	{L"tdmodem", NULL, TDMODEM_SIZE, TDMODEM_ADR},
	{L"tddsp", NULL, TDDSP_SIZE, TDDSP_ADR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_GSM
static boot_image_required_t const s_boot_image_gsm_table[] = {
	{L"g_fixnv1", L"g_fixnv2", GSM_FIXNV_SIZE, GSM_FIXNV_ADDR},
	{L"g_runtimenv1", L"g_runtimenv2", GSM_RUNNV_SIZE, GSM_RUNNV_ADDR},
	{L"g_modem", NULL, GSM_MODEM_SIZE, GSM_MODEM_ADDR},
	{L"g_dsp", NULL, GSM_DSP_SIZE, GSM_DSP_ADDR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_W
static boot_image_required_t const s_boot_image_W_table[] = {
	{L"wfixnv1", L"wfixnv2", FIXNV_SIZE, WFIXNV_ADR},
	{L"wruntimenv1", L"wruntimenv2", RUNTIMENV_SIZE, WRUNTIMENV_ADR},
	{L"wmodem", NULL, WMODEM_SIZE, WMODEM_ADR},
	{L"wdsp", NULL, WDSP_SIZE, WDSP_ADR},
	{NULL, NULL, 0, 0}
};
#endif

#ifdef CONFIG_SUPPORT_WIFI
static boot_image_required_t const s_boot_image_WIFI_table[] = {
	{L"wcnfixnv1", L"wcnfixnv2", FIXNV_SIZE, WCNFIXNV_ADR},
	{L"wcnruntimenv1", L"wcnruntimenv2", RUNTIMENV_SIZE, WCNRUNTIMENV_ADR},
	{L"wcnmodem", NULL, WCNMODEM_SIZE, WCNMODEM_ADR},
	{NULL, NULL, 0, 0}
};
#endif

static boot_image_required_t const s_boot_image_COMMON_table[] = {
#if !BOOT_NATIVE_LINUX
	{L"vm", NULL, VMJALUNA_SIZE, VMJALUNA_ADR},
#endif
#ifdef CONFIG_SIMLOCK_ENABLE
	{L"simlock", NULL, SIMLOCK_SIZE, SIMLOCK_ADR},
#endif
#ifdef CONFIG_DFS_ENABLE
	{L"pm_sys", NULL, DFS_SIZE, DFS_ADDR},
#endif
	{NULL, NULL, 0, 0}

};

static boot_image_required_t *const s_boot_image_table[] = {
#ifdef CONFIG_SUPPORT_TDLTE
	s_boot_image_tl_table,
#endif

#ifdef CONFIG_SUPPORT_WLTE
	s_boot_image_wl_table,
#endif

#ifdef CONFIG_SUPPORT_LTE
	s_boot_image_lte_table,
#endif

#ifdef CONFIG_SUPPORT_GSM
	s_boot_image_gsm_table,
#endif

#ifdef CONFIG_SUPPORT_TD
	s_boot_image_TD_table,
#endif

#ifndef CONFIG_TIZEN
#ifdef CONFIG_SUPPORT_W
	s_boot_image_W_table,
#endif
#endif

#ifndef CONFIG_TIZEN
#ifdef CONFIG_SUPPORT_WIFI
	s_boot_image_WIFI_table,
#endif
#endif
	s_boot_image_COMMON_table,

	0
};

#ifdef CONFIG_SECURE_BOOT
uint8 header_buf[SEC_HEADER_MAX_SIZE];
#endif

int read_logoimg(char *bmp_img, size_t size)
{
	block_dev_desc_t *p_block_dev = NULL;
	disk_partition_t info;

	p_block_dev = get_dev("mmc", 1);
	if (NULL == p_block_dev) {
		return -1;
	}
	if (!get_partition_info_by_name(p_block_dev, L"logo", &info)) {
		if (TRUE != Emmc_Read(PARTITION_USER, info.start, size / EMMC_SECTOR_SIZE, bmp_img)) {
			debugf("function: %s nand read error\n", __FUNCTION__);
			return -1;
		}
	}
	return 0;
}

int read_spldata()
{
#if 0
	int size = CONFIG_SPL_LOAD_LEN;
	if (TRUE != Emmc_Read(PARTITION_BOOT1, 0, size / EMMC_SECTOR_SIZE, (uint8 *) spl_data)) {
		debugf("vmjaluna nand read error \n");
		return -1;
	}
#endif
	return 0;
}

#ifdef CONFIG_SECURE_BOOT

int get_spl_hash(void *hash_data)
{
	NBLHeader *header;
	int len;
	uint8 *spl_data;
	int ret = 0;
	//int size = CONFIG_SPL_LOAD_LEN;
	int size = CONFIG_SPL_HASH_LEN;

	spl_data = malloc(size);
	if (!spl_data) {
		return ret;
	}

	if (TRUE != Emmc_Read(PARTITION_BOOT1, 0, size / EMMC_SECTOR_SIZE, (uint8 *) spl_data)) {
		debugf("PARTITION_BOOT1 read error \n");
		return ret;
	}

	header = (NBLHeader *) ((unsigned char *)spl_data + BOOTLOADER_HEADER_OFFSET);
	len = header->mHashLen;
	/*clear header */
	memset(header, 0, sizeof(NBLHeader));
	header->mHashLen = len;
	debugf("cal spl hash len=%d\n", header->mHashLen * 4);

	ret = cal_sha1(spl_data, (header->mHashLen) << 2, hash_data);

	if (spl_data)
		free(spl_data);

	return ret;
}
#endif

/**
	just convert partition name wchar to char with violent.
*/
LOCAL __inline char *w2c(wchar_t * wchar)
{
	static char buf[72] = { 0 };
	unsigned int i = 0;
	while ((NULL != wchar[i]) && (i < 72)) {
		buf[i] = wchar[i] & 0xFF;
		i++;
	}
	buf[i] = 0;

	return buf;
}

LOCAL void _boot_secure_check(void)
{
#ifdef CONFIG_SECURE_BOOT
	int puk_adr;
	vlr_info_t *vlr_info;

#ifdef CONFIG_SUPPORT_W
	puk_adr = CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_SIZ;
	vlr_info = (vlr_info_t *) (WDSP_ADR + WDSP_SIZE - VLR_INFO_SIZ);
	secure_check(WDSP_ADR, vlr_info->length, vlr_info, puk_adr);

	puk_adr = CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_SIZ;
	vlr_info = (vlr_info_t *) (WMODEM_ADR + WMODEM_SIZE - VLR_INFO_SIZ);
	secure_check(WMODEM_ADR, vlr_info->length, vlr_info, puk_adr);
#endif

#ifdef CONFIG_SUPPORT_WIFI
	puk_adr = CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_SIZ;
	vlr_info = (vlr_info_t *) (WCNMODEM_ADR + WCNMODEM_SIZE - VLR_INFO_SIZ);
	secure_check(WCNMODEM_ADR, vlr_info->length, vlr_info, puk_adr);
#endif

#if !BOOT_NATIVE_LINUX
	secure_check(VMJALUNA_ADR, 0, VMJALUNA_ADR + VMJALUNA_SIZE - VLR_INFO_OFF,
		     CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_OFF);
#endif

#ifdef CONFIG_SIMLOCK
	secure_check(SIMLOCK_ADR, 0, SIMLOCK_ADR + SIMLOCK_SIZE - VLR_INFO_OFF,
		     CONFIG_SYS_NAND_U_BOOT_DST + CONFIG_SYS_NAND_U_BOOT_SIZE - KEY_INFO_SIZ - VLR_INFO_OFF);
#endif
#endif
	return;
}

/**
	Function for reading user partition.
*/
PUBLIC int _boot_partition_read(block_dev_desc_t * dev, wchar_t * partition_name, u32 offsetsector, u32 size, u8 * buf)
{
	int ret = 0;
	u32 left;
	u32 nsct;
	char *sctbuf = NULL;
	disk_partition_t info;

	if (NULL == buf) {
		debugf("%s:buf is NULL!\n", __func__);
		goto end;
	}
	nsct = size / EMMC_SECTOR_SIZE;
	left = size % EMMC_SECTOR_SIZE;

	if (get_partition_info_by_name(dev, partition_name, &info)) {
		debugf("get partition %s info failed!\n", w2c(partition_name));
		goto end;
	}

	if (TRUE != Emmc_Read(PARTITION_USER, info.start + offsetsector, nsct, buf))
		goto end;

	if (left) {
		sctbuf = malloc(EMMC_SECTOR_SIZE);
		if (NULL != sctbuf) {
			if (TRUE == Emmc_Read(PARTITION_USER, info.start + offsetsector + nsct, 1, sctbuf)) {
				memcpy(buf + (nsct * EMMC_SECTOR_SIZE), sctbuf, left);
				ret = 1;
			}
			free(sctbuf);
		}
	} else {
		ret = 1;
	}

end:
	debugf("%s: partition %s read %s!\n", __func__, w2c(partition_name), ret ? "success" : "failed");
	return ret;
}

PUBLIC int blk_data_read(u32 offset, u32 size, u8 *buf)
{
	int ret = 0;
	char *sctbuf = NULL;
	u32 start_sec, nsct, left;

	if (!buf) {
		debugf("NULL BUF\n");
		goto end;
	}

	start_sec = offset / EMMC_SECTOR_SIZE;
	nsct = size / EMMC_SECTOR_SIZE;
	left = size % EMMC_SECTOR_SIZE;

	if (nsct && !Emmc_Read(PARTITION_USER, start_sec, nsct, buf)) {
		debugf("Failed to read mmc\n");
		goto end;
	}

	if (left) {
		sctbuf = malloc(EMMC_SECTOR_SIZE);
		if (sctbuf) {
			if (!Emmc_Read(PARTITION_USER, start_sec + nsct, 1, sctbuf)) {
				debugf("Failed to read mmc\n");
				goto end;
			}

			memcpy(buf + (nsct * EMMC_SECTOR_SIZE), sctbuf, left);
			ret = 1;
			free(sctbuf);
		}
	} else {
		ret = 1;
	}

end:
	return ret;
}

/**
	Function for writing user partition.
*/
PUBLIC int _boot_partition_write(block_dev_desc_t * dev, wchar_t * partition_name, u32 size, u8 * buf)
{
	disk_partition_t info;

	if (NULL == buf) {
		debugf("%s:buf is NULL!\n", __FUNCTION__);
		return 0;
	}
	size = (size + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1));
	size = size / EMMC_SECTOR_SIZE;
	if (0 == get_partition_info_by_name(dev, partition_name, &info)) {
		if (TRUE != Emmc_Write(PARTITION_USER, info.start, size, buf)) {
			debugf("%s: partition:%s read error!\n", __FUNCTION__, w2c(partition_name));
			return 0;
		}
	} else {
		debugf("%s: partition:%s >>>get partition info failed!\n", __FUNCTION__, w2c(partition_name));
		return 0;
	}
	debugf("%s: partition:%s write success!\n", __FUNCTION__, w2c(partition_name));
	return 1;
}

/**
	Function for displaying logo.
*/
LOCAL __inline void _boot_display_logo(block_dev_desc_t * dev, int backlight_set)
{
	size_t size;

#if defined(CONFIG_LCD_720P) || defined(CONFIG_LCD_HD) || CONFIG_LCD_PAD_WXGA	//LiWei add CONFIG_LCD_HD
	size = 1 << 20;
#else
	size = 1 << 19;
#endif
	uint8 *bmp_img = malloc(size);
	if (!bmp_img) {
		debugf("%s: malloc for splash image failed!\n", __FUNCTION__);
		return;
	}
	if (!_boot_partition_read(dev, L"logo", 0, size, bmp_img)) {
		debugf("%s: read logo partition failed!\n", __FUNCTION__);
		goto end;
	}
	lcd_display_logo(backlight_set, (ulong) bmp_img, size);
end:
	free(bmp_img);
	return;
}

/**
	we assume partition with backup must check ecc.
*/
LOCAL __inline int _boot_read_partition_with_backup(block_dev_desc_t * dev, boot_image_required_t info)
{
	uint8 *bakbuf = NULL;
	uint8 *oribuf = NULL;
	u8 status = 0;
	uint8 header[EMMC_SECTOR_SIZE];
	uint32 checksum = 0;
	nv_header_t *header_p = NULL;
	uint32 bufsize = info.size + EMMC_SECTOR_SIZE;

	header_p = header;
	bakbuf = malloc(bufsize);
	if (NULL == bakbuf)
		return 0;
	memset(bakbuf, 0xff, bufsize);
	oribuf = malloc(bufsize);
	if (NULL == oribuf) {
		free(bakbuf);
		return 0;
	}
	memset(oribuf, 0xff, bufsize);

	if (_boot_partition_read(dev, info.partition, 0, info.size + EMMC_SECTOR_SIZE, oribuf)) {
		memset(header, 0, EMMC_SECTOR_SIZE);
		memcpy(header, oribuf, EMMC_SECTOR_SIZE);
		checksum = header_p->checksum;
		debugf("_boot_read_partition_with_backup origin checksum 0x%x\n", checksum);
		if (_chkNVEcc(oribuf + EMMC_SECTOR_SIZE, info.size, checksum)) {
			memcpy(info.mem_addr, oribuf + EMMC_SECTOR_SIZE, info.size);
			status += 1;
		}
	}
	if (_boot_partition_read(dev, info.bak_partition, 0, info.size + EMMC_SECTOR_SIZE, bakbuf)) {
		memset(header, 0, EMMC_SECTOR_SIZE);
		memcpy(header, bakbuf, EMMC_SECTOR_SIZE);
		checksum = header_p->checksum;
		debugf("_boot_read_partition_with_backup backup checksum 0x%x\n", checksum);
		if (_chkNVEcc(bakbuf + EMMC_SECTOR_SIZE, info.size, checksum))
			status += 1 << 1;
	}

	switch (status) {
	case 0:
		debugf("%s:(%s)both org and bak partition are damaged!\n", __FUNCTION__, w2c(info.partition));
    	memset(info.mem_addr, 0, info.size);
		free(bakbuf);
		free(oribuf);
		return 0;
	case 1:
		debugf("%s:(%s)bak partition is damaged!\n", __FUNCTION__, w2c(info.bak_partition));
		_boot_partition_write(dev, info.bak_partition, info.size + EMMC_SECTOR_SIZE, oribuf);
		break;
	case 2:
		debugf("%s:(%s)org partition is damaged!\n!", __FUNCTION__, w2c(info.partition));
		memcpy(info.mem_addr, bakbuf + EMMC_SECTOR_SIZE, info.size);
		_boot_partition_write(dev, info.partition, info.size + EMMC_SECTOR_SIZE, bakbuf);
		break;
	case 3:
		debugf("%s:(%s)both org and bak partition are ok!\n", __FUNCTION__, w2c(info.partition));
		break;
	default:
		debugf("%s: status error!\n", __FUNCTION__);
		free(bakbuf);
		free(oribuf);
		return 0;
	}
	free(bakbuf);
	free(oribuf);
	return 1;
}

/**
	Function for reading image which is needed when power on.
*/
//LOCAL __inline
int _boot_load_required_image(block_dev_desc_t * dev, boot_image_required_t img_info)
{
	uint32 secure_boot_offset = 0;

	debugf("%s: load %s to addr 0x%08x\n", __FUNCTION__, w2c(img_info.partition), img_info.mem_addr);

	if (NULL != img_info.bak_partition) {
		_boot_read_partition_with_backup(dev, img_info);
	} else {
#ifdef CONFIG_SECURE_BOOT
		if (!_boot_partition_read(dev, img_info.partition, 0, SEC_HEADER_MAX_SIZE, header_buf)) {
			debugf("%s:%s read error!\n", __FUNCTION__, w2c(img_info.partition));
			return 0;
		}
		//if(header_parser(header_buf) )
		secure_boot_offset = get_code_offset(header_buf);
		_boot_partition_read(dev, img_info.partition, 0 + secure_boot_offset, img_info.size, (u8 *) img_info.mem_addr);

		secure_verify(L"uboot", header_buf, img_info.mem_addr);
#else
		_boot_partition_read(dev, img_info.partition, 0, img_info.size, (u8 *) img_info.mem_addr);
#endif
	}

	return 1;
}

/**
	Function for checking and loading kernel/ramdisk image.
*/
LOCAL int _boot_load_kernel_ramdisk_image(block_dev_desc_t * dev, char *bootmode, boot_img_hdr * hdr)
{
	wchar_t *partition = NULL;
	uint32 size, offset;
	uint32 dt_img_adr;
	uint32 secure_boot_offset = 0;

	if (0 == memcmp(bootmode, RECOVERY_PART, strlen(RECOVERY_PART))) {
		partition = L"recovery";
		debugf("enter recovery mode!\n");
	} else {
		partition = BOOT_PART;
		debugf("Enter boot mode (partition name: %s)\n", partition);
	}

	if (!blk_data_read(0x2C00000, sizeof(*hdr), hdr)) {
		debugf("%s:%s read error!\n", __FUNCTION__, w2c(partition));
		return 0;
	}

	//image header check
	if (hdr->magic_num != MAGIC_NUM) {
		debugf("BAD BOOT IMAGE HEADER: %x\n", hdr->magic_num);
		return 0;
	}
	debugf("BOOT IMAGE HEADER: %x\n", hdr->magic_num);

	//read kernel image
	size = roundup(sizeof(*hdr), ALIGN_SIZE) +
		roundup(hdr->kernel_size, ALIGN_SIZE) +
		roundup(hdr->dt_size, ALIGN_SIZE);

	debugf("bzImage size: %x\n", size);

	if (!blk_data_read(0x2C00000, size, KERNEL_ADR - roundup(sizeof(*hdr), ALIGN_SIZE))) {
		debugf("%s:%s kernel read error!\n", __FUNCTION__, w2c(partition));
		return 0;
	}

	//read dt image
	dt_img_adr = KERNEL_ADR + roundup(hdr->kernel_size, ALIGN_SIZE);
	debugf("dt_img_adr: %u\n", dt_img_adr);
	debugf("dt_img_adr: %x\n", dt_img_adr);
	if (load_dtb((int)DT_ADR, (void *)dt_img_adr)) {
		debugf("%s:dt load error!\n", __FUNCTION__);
		return 0;
	}
#ifdef CONFIG_SDRAMDISK
	{
		int sd_ramdisk_size = 0;
#ifdef WDSP_ADR
		size = WDSP_ADR - RAMDISK_ADR;
#else
		size = TDDSP_ADR - RAMDISK_ADR;
#endif
		if (size > 0)
			sd_ramdisk_size = load_sd_ramdisk((uint8 *) RAMDISK_ADR, size);
		if (sd_ramdisk_size > 0)
			hdr->ramdisk_size = sd_ramdisk_size;
	}
#endif
	return 1;
}

#ifdef CONFIG_SECURE_BOOT
PUBLIC int secure_verify_partition(block_dev_desc_t * dev, wchar_t * partition_name, int ram_addr)
{
	int ret = 0;
	int size;
	disk_partition_t info;

	if (get_partition_info_by_name(dev, partition_name, &info)) {
		debugf("verify get partition %s info failed!\n", w2c(partition_name));
		ret = 1;
	}
	size = info.size * GPT_BLOCK_SIZE;
	debugf("%s=%x  =%x\n", w2c(partition_name), info.size, size);

	_boot_partition_read(dev, partition_name, 0, size, (u8 *) ram_addr);
	secure_verify(L"uboot", ram_addr, 0);
	return ret;
}
#endif

void vlx_nand_boot(char *kernel_pname, char *cmdline, int backlight_set)
{
	boot_img_hdr *hdr = (void *)raw_header;
	block_dev_desc_t *dev = NULL;
	wchar_t *partition = NULL;
	int i, j;
	long long start = get_ticks();

	dev = get_dev("mmc", 1);
	if (NULL == dev) {
		debugf("Fatal Error,get_dev mmc failed!\n");
		return;
	}
#ifdef CONFIG_SC9630
	pmic_arm7_RAM_active();
#endif

#ifndef CONFIG_TIZEN
#ifdef CONFIG_SPLASH_SCREEN
	_boot_display_logo(dev, backlight_set);
	performance_debug("7:");
#endif
#endif
	set_vibrator(FALSE);

#if ((!BOOT_NATIVE_LINUX)||(BOOT_NATIVE_LINUX_MODEM))
	//load required image which config in table
	i = 0;
	while (s_boot_image_table[i]) {
		j = 0;
		while (s_boot_image_table[i][j].partition) {
			_boot_load_required_image(dev, s_boot_image_table[i][j]);
			j++;
		}
		i++;
	}
	performance_debug("8:");
#endif

#ifdef CONFIG_SECURE_BOOT
	if (0 == memcmp(kernel_pname, RECOVERY_PART, strlen(RECOVERY_PART))) {
		partition = L"recovery";
	} else {
		partition = L"boot";
	}
	secure_verify_partition(dev, partition, KERNEL_ADR);
#endif
	//loader kernel and ramdisk
	if (!_boot_load_kernel_ramdisk_image(dev, kernel_pname, hdr))
		return;
	performance_debug("9:");
	load_image_time = get_ticks() - start;
	//secure check for secure boot
	//_boot_secure_check();

	if (creat_cmdline(cmdline, hdr)) {
		debugf("creat_cmdline failed\n");
		return;
	}
#if BOOT_NATIVE_LINUX_MODEM
	//sipc addr clear
	sipc_addr_reset();
#endif
#if defined CONFIG_SC9630
//      Emmc_DisSdClk();
#endif
	vlx_entry();
}
