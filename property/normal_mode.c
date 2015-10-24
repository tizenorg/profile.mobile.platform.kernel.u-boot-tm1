#include "normal_mode.h"
#include "calibration_detect.h"
#include <mmc.h>
#include <fat.h>
#include <asm/arch/sprd_reg.h>
#ifdef CONFIG_ARCH_SCX35L	//only for sharkL branch modem boot process
#include <asm/arch/cp_boot.h>
#endif
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

#ifdef CONFIG_TIZEN
#include "tizen_misc.h"
#endif

#include <common.h>
DECLARE_GLOBAL_DATA_PTR;

//#define SPRD_BM_UBOOT_SET
#define FACTORY_PART "prodnv"
#define CMDLINE_BUF_SIZE	(1024)

unsigned spl_data_buf[0x2000] __attribute__ ((align(4))) = {
0};

unsigned harsh_data_buf[8] __attribute__ ((align(4))) = {
0};

void *spl_data = spl_data_buf;
void *harsh_data = harsh_data_buf;
unsigned char raw_header[8192];
const int SP09_MAX_PHASE_BUFF_SIZE = sizeof(SP09_PHASE_CHECK_T);
unsigned int g_charger_mode = 0;
char serial_number_to_transfer[SP09_MAX_SN_LEN];

extern int charger_connected(void);

extern void *lcd_base;
#ifdef CONFIG_OF_LIBFDT
static char boot_cmd[64];
#endif

#ifdef SPRD_BM_UBOOT_SET
/**
 *	sprd_bm_set_uboot_reg - set uboot bus monitor reg
 *	How to use this interface.:
 *	1. set the channel config you want to monitor the mem, e.g, channel 0-9, bm_cnt=0, bm_cnt<10.
 *	2. set the addr range, e.g 80000000-90000000,
 *		0x30040008 + bm_cnt * 0x10000) = 0x80000000;
 *		0x3004000C + bm_cnt * 0x10000) = 0x90000000;
 *	3. if you want mask the addr, just like the following code.
 *	4. please do not change others reg.
 *	5. if you make this config active, you must disable the bus monitor in kernel side.
 */
static void sprd_bm_set_uboot_reg(void)
{
	u8 bm_cnt;
	for (bm_cnt = 0; bm_cnt < 10; bm_cnt++) {
		//if((bm_cnt == 5)||(bm_cnt == 6))//||(bm_cnt == 0)||(bm_cnt == 1)||(bm_cnt == 3))
		//      continue;
		*(volatile u32 *)(0x30040000 + bm_cnt * 0x10000) = 0x20000001;
		*(volatile u32 *)(0x30040000 + bm_cnt * 0x10000) = 0x10000001;
		*(volatile u32 *)(0x30040004 + bm_cnt * 0x10000) = 0x00000003;
		*(volatile u32 *)(0x30040008 + bm_cnt * 0x10000) = 0x00620000;
		*(volatile u32 *)(0x3004000c + bm_cnt * 0x10000) = 0x077ffffc;
		*(volatile u32 *)(0x30040010 + bm_cnt * 0x10000) = 0xC0000000;
		*(volatile u32 *)(0x30040014 + bm_cnt * 0x10000) = 0x0FFFFFFF;
		*(volatile u32 *)(0x30040018 + bm_cnt * 0x10000) = 0x0FFFFFFF;
		*(volatile u32 *)(0x3004001c + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040020 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040024 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040028 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x3004002c + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040030 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040034 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040038 + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x3004003c + bm_cnt * 0x10000) = 0x00000000;
		*(volatile u32 *)(0x30040040 + bm_cnt * 0x10000) = 0x00000000;
	}
}
#endif

unsigned short calc_checksum(unsigned char *dat, unsigned long len)
{
	unsigned short num = 0;
	unsigned long chkSum = 0;
	while (len > 1) {
		num = (unsigned short)(*dat);
		dat++;
		num |= (((unsigned short)(*dat)) << 8);
		dat++;
		chkSum += (unsigned long)num;
		len -= 2;
	}
	if (len) {
		chkSum += *dat;
	}
	chkSum = (chkSum >> 16) + (chkSum & 0xffff);
	chkSum += (chkSum >> 16);
	return (~chkSum);
}

unsigned char _chkNVEcc(uint8_t * buf, uint32_t size, uint32_t checksum)
{
	uint16_t crc;

	crc = calc_checksum(buf, size);
	debugf("_chkNVEcc calcout 0x%lx, org 0x%llx\n", crc, checksum);
	return (crc == (uint16_t) checksum);
}

#ifdef CONFIG_SECURE_BOOT
//#define PRIMPUKPATH "/dev/block/mmcblk0boot0"
//#define PRIMPUKSTART 512
//#define PRIMPUKLEN 260
void fdt_secureboot_param(fdt_blob)
{

	char *buf = NULL;
	int str_len = 0;
	buf = malloc(CMDLINE_BUF_SIZE);
	memset(buf, 0, CMDLINE_BUF_SIZE);

#ifdef PRIMPUKPATH
	str_len = strlen(buf);
	sprintf(&buf[str_len], " primpukpath=%s", PRIMPUKPATH);	//"/dev/block/mmcblk0boot0");
	str_len = strlen(buf);
	buf[str_len] = '\0';
#endif
#ifdef PRIMPUKSTART
	str_len = strlen(buf);
	sprintf(&buf[str_len], " primpukstart=%d", PRIMPUKSTART);	//512);
	str_len = strlen(buf);
	buf[str_len] = '\0';
#endif
#ifdef PRIMPUKLEN
	str_len = strlen(buf);
	sprintf(&buf[str_len], " primpuklen=%d", PRIMPUKLEN);	//260);
	str_len = strlen(buf);
	buf[str_len] = '\0';
#endif
	/*hash write by pc tool,but the hash value calculated by u-boot */
	/*if rom secure enable,do not need cal spl hash and pass to kernel */
	if (!secureboot_enabled()) {

	int ret = get_spl_hash(harsh_data);
	if (ret) {
		str_len = strlen(buf);
		sprintf(&buf[str_len], " securesha1=%08x%08x%08x%08x%08x", *(uint32_t *) harsh_data, *(uint32_t *) (harsh_data + 4),
			*(uint32_t *) (harsh_data + 8), *(uint32_t *) (harsh_data + 12), *(uint32_t *) (harsh_data + 16));
		str_len = strlen(buf);
		buf[str_len] = '\0';
	}
}
	fdt_chosen_bootargs_append(fdt_blob, buf, 1);
}
#endif

#ifdef CONFIG_TIZEN
int fdt_fixup_for_tizen(void *fdt)
{
	char buf[1024];
	unsigned char uid[16];
	int ret, nodeoffset, str_len;
	char *ptr = buf;
	char *s;
	unsigned int val;
	struct mmc *mmc;

	/* Tizen default cmdline: mem */
	ptr += sprintf(ptr, CMDLINE_DEFAULT_TIZEN);

	val = tizen_get_part_num(PARTS_ROOTFS);
	ptr += sprintf(ptr, " root=/dev/mmcblk0p%d ro rootfstype=ext4 rootwait", val);

	ptr += sprintf(ptr, " lcd_id=ID%06x", load_lcd_id_to_kernel());
	ptr += sprintf(ptr, " lcd_base=%x", CONFIG_FB_RAM_BASE);

	/* check ramdisk_size */
	ptr += sprintf(ptr, " initrd=0x%x,0x%x", RAMDISK_ADR, 0);

	ptr += sprintf(ptr, " mtp_offset=%s", load_mtp_offset_to_kernel());
	ptr += sprintf(ptr, " elvss_offset=0x%x", load_elvss_offset_to_kernel());
	ptr += sprintf(ptr, " hbm_offset=%s", load_hbm_offset_to_kernel());

	ptr += sprintf(ptr, " wfixnv=0x%x,0x%x", WFIXNV_ADR, FIXNV_SIZE);
	ptr += sprintf(ptr, " wruntimenv=0x%x,0x%x", WRUNTIMENV_ADR, RUNTIMENV_SIZE);

	switch (check_pm_status()) {
	case PM_STATE_LPM:
		ptr += sprintf(ptr, " systemd.unit=charging-mode.target");
		/* Write Charger state */
		*(volatile unsigned int *)0xFC8 = 0x03;
		sprdchg_start_charge();
		break;
	case PM_STATE_NORMAL:
	default:
		ptr += sprintf(ptr, " bootmode=normal");
	}
	thor_save_env("normal");

	s = getenv("hw_revision");
	if (s != NULL)
		val = (u32) simple_strtoul(s, NULL, 10);
	else
		val = 0;
	ptr += sprintf(ptr, " hw_revision=%d", val);

	s = getenv("muic_rustproof");
	if (s != NULL)
		val = (u32) simple_strtoul(s, NULL, 10);
	else
		val = 0;
	ptr += sprintf(ptr, " muic_rustproof=%d", val);

	s = getenv("dbg_level");
	if (s && (*s == 'a')) {
		ptr += sprintf(ptr, " sec_debug.enable=1");
		ptr += sprintf(ptr, " sec_debug.enable_user=0");
	} else if (s && (*s == 'h')) {
		ptr += sprintf(ptr, " sec_debug.enable=1");
		ptr += sprintf(ptr, " sec_debug.enable_user=1");
	} else if (s && (*s == 'm')) {
		ptr += sprintf(ptr, " sec_debug.enable=1");
		ptr += sprintf(ptr, " sec_debug.enable_user=0");
	} else {
		ptr += sprintf(ptr, " sec_debug.enable=0");
		ptr += sprintf(ptr, " sec_debug.enable_user=0");
	}

	if (tizen_get_jig_state() == 2) {
		s = getenv("console");
		if (s && (*s == 'o'))
			ptr += sprintf(ptr, " console=ttyS1,115200n8 loglevel=7");
		else
			ptr += sprintf(ptr, " console=ram loglevel=0");
	} else {
			ptr += sprintf(ptr, " console=ram loglevel=0");
	}

	s = getenv("sec_log");
	if (s && (*s == 'o')) {
		if (s = getenv("sec_log_addr")) {
			val = (u32) simple_strtoul(s, NULL, 16);
			ptr += sprintf(ptr, " sec_log=0x%x@0x%x", SEC_LOG_LENGTH, val);
		}
	}

	ptr += sprintf(ptr, " bootloader.ver=%s", CONFIG_BOOTLOADER_VER);

	s = getenv("emmc_checksum");
	if (s != NULL)
		val = (u32) simple_strtoul(s, NULL, 10);
	else
		val = 0;

	ptr += sprintf(ptr, " tizenboot.emmc_checksum=%d", val);

	ptr += sprintf(ptr, " mem_cs=%d, mem_cs0_sz=%08x",get_dram_cs_number(), get_dram_cs0_size());

	/* TODO: connie, cordon */
	/* connie, cordon */
	/* toss ddi value for sysscope */
	//	set_system_info(CONFIG_MODEL_NAME, CONFIG_OPERATOR, CONFIG_REGION, VT_SPRD, 16);
	//	ptr += sprintf(ptr, " connie=%s", get_conniecmdline_value());
	//	ptr += sprintf(ptr, " cordon=%s", get_ddicmdline_value());

	tizen_get_emmc_serial_number((unsigned int *)uid);
	ptr += sprintf(ptr, " tizenboot.serialno=%x%08x",
			*(unsigned int *)(uid + 8), *(unsigned int *)(uid + 12));

	/* if is uart calibraton, remove ttys1 console */
	if (is_calibration_by_uart())
		val = 1;
	else
		val = 0;

	ptr += sprintf(ptr, " calmode=%d", is_calibration_by_uart());
	ptr += sprintf(ptr, " fgu_init=%d,%d", get_fgu_vol(), get_fgu_cur());

	str_len = strlen(buf);
	buf[str_len] = '\0';

	nodeoffset = fdt_path_offset (fdt, "/chosen");
	ret = fdt_setprop_string(fdt, nodeoffset, "bootargs", buf);
	return ret;
}
#endif

/*FDT_ADD_SIZE used to describe the size of the new bootargs items*/
/*include lcd id, lcd base, etc*/
#define FDT_ADD_SIZE (1024)
static int start_linux()
{
	void (*theKernel) (int zero, int arch, u32 params);
	u32 exec_at = (u32) - 1;
	u32 parm_at = (u32) - 1;
	u32 machine_type;
	u8 *fdt_blob;
	u32 fdt_size;
	boot_img_hdr *hdr = raw_header;
	int err;

	machine_type = machine_arch_type;	/* get machine type */
	machine_type = 2014;	/* get machine type */
	theKernel = (void (*)(int, int, u32))KERNEL_ADR;	/* set the kernel address */
#if !(defined(CONFIG_SC8830) || defined(CONFIG_SC9630))
	*(volatile u32 *)0x84001000 = 'j';
	*(volatile u32 *)0x84001000 = 'm';
	*(volatile u32 *)0x84001000 = 'p';
#endif

#ifdef CONFIG_OF_LIBFDT
	fdt_blob = (u8 *) DT_ADR;
	if (fdt_check_header(fdt_blob) != 0) {
		printk("image is not a fdt\n");
	}
	fdt_size = fdt_totalsize(fdt_blob);

	err = fdt_open_into(fdt_blob, fdt_blob, fdt_size + FDT_ADD_SIZE);
	if (err != 0) {
		printf("libfdt fdt_open_into(): %s\n", fdt_strerror(err));
	}

#ifdef CONFIG_TIZEN
	load_nvitem();
	load_modem_data();
	load_dsp_data();

	sipc_addr_reset();
	modem_entry();

	fdt_fixup_for_tizen(fdt_blob);
#ifdef CONFIG_EMMC_BOOT
	Emmc_DisSdClk();
#endif

	theKernel(0, machine_type, (unsigned long)fdt_blob);
	while (1);
#endif	/* CONFIG_TIZEN */

//#else
//	fdt_initrd_norsvmem(fdt_blob, RAMDISK_ADR, RAMDISK_ADR + hdr->ramdisk_size, 1);
	fdt_fixup_lcdid(fdt_blob);
	fdt_fixup_lcdbase(fdt_blob);
	fdt_fixup_calibration_parameter(fdt_blob);
	fdt_fixup_serialno(fdt_blob);
	fdt_fixup_adc_calibration_data(fdt_blob);
	fdt_fixup_dram_training(fdt_blob);
	fdt_fixup_ddr_size(fdt_blob);
#ifdef CONFIG_SECURE_BOOT
	fdt_secureboot_param(fdt_blob);
#endif
#ifndef CONFIG_EMMC_BOOT
	fdt_fixup_mtd(fdt_blob);
#endif
	debugf("start_linux boot_cmd: %s\n", boot_cmd);
	fdt_fixup_boot_mode(fdt_blob, boot_cmd);
	fdt_fixup_boot_ram_log(fdt_blob);
	fdt_fixup_chosen_bootargs_board_private(fdt_blob, boot_cmd);
#ifdef SPRD_BM_UBOOT_SET
	sprd_bm_set_uboot_reg();
#endif
	// start modem CP
	modem_entry();
#ifdef CONFIG_EMMC_BOOT
	Emmc_DisSdClk();
#endif
	theKernel(0, machine_type, (unsigned long)fdt_blob);
#else	/* CONFIG_OF_LIBFDT */
	// start modem CP
	modem_entry();
#ifdef CONFIG_EMMC_BOOT
	Emmc_DisSdClk();
#endif
	theKernel(0, machine_type, DT_ADR);	/* jump to kernel with register set */
#endif

	while (1);
	return 0;
}

void lcd_display_logo(int backlight_set, ulong bmp_img, size_t size)
{
#define mdelay(t)     ({unsigned long msec=(t); while (msec--) { udelay(1000);}})	//LiWei add
#ifdef CONFIG_SPLASH_SCREEN
	extern int lcd_display_bitmap(ulong bmp_image, int x, int y);
	extern void lcd_display(void);
	extern void *lcd_base;
	extern void Dcache_CleanRegion(unsigned int addr, unsigned int length);
	extern void set_backlight(uint32_t value);
	if (backlight_set == BACKLIGHT_ON) {
		lcd_display_bitmap((ulong) bmp_img, 0, 0);
#if defined(CONFIG_SC8810) || defined(CONFIG_SC8825) || defined(CONFIG_SC8830) || defined(CONFIG_SC9630)
		Dcache_CleanRegion((unsigned int)(lcd_base), size << 1);	//Size is to large.
#endif
		lcd_display();
#ifdef CONFIG_SC8830_LVDS
		mdelay(50);	//LiWei add
#endif
		mdelay(50);
		set_backlight(255);
	} else {
		memset((unsigned int)lcd_base, 0, size);
#if defined(CONFIG_SC8810) || defined(CONFIG_SC8825) || defined(CONFIG_SC8830) || defined(CONFIG_SC9630)
		Dcache_CleanRegion((unsigned int)(lcd_base), size << 1);	//Size is to large.
#endif
		lcd_display();
	}
#endif
}

int is_factorymode()
{
	char factorymode_falg[8] = { 0 };
	int ret = 0;

	if (do_fs_file_read(FACTORY_PART, "/factorymode.file", factorymode_falg, 8))
		return 0;
	debugf("Checking factorymode :  factorymode_falg = %s \n", factorymode_falg);
	if (!strcmp(factorymode_falg, "1"))
		ret = 1;
	else
		ret = 0;
	debugf("Checking factorymode :  ret = %d \n", ret);
	return ret;
}

char *get_product_sn(void)
{
	SP09_PHASE_CHECK_T phase_check;

	memset(serial_number_to_transfer, 0x0, SP09_MAX_SN_LEN);

	strcpy(serial_number_to_transfer, "0123456789ABCDEF");
	if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(phase_check), 0, (char *)&phase_check)) {
		debugf("%s: read miscdata error.\n", __func__);
		return serial_number_to_transfer;
	}

	if ((phase_check.Magic == SP09_SPPH_MAGIC_NUMBER) && strlen(phase_check.SN1)) {
		memcpy(serial_number_to_transfer, phase_check.SN1, SP09_MAX_SN_LEN);
	}

	return serial_number_to_transfer;
}

#ifdef CONFIG_OF_LIBFDT
static char *set_boot_mode(char *cmdline)
{
	char *boot_mode_p = NULL;

	memset(boot_cmd, 0, 64);
	boot_mode_p = strstr(cmdline, "androidboot");
	if (boot_mode_p) {
		if (strlen(boot_mode_p) > 64) {
			debugf("boot mode too long\n");
			return NULL;
		}
		strcpy(boot_cmd, boot_mode_p);
	}
	debugf("CONFIG_OF_LIBFDT cmdline %s boot_cmd %s\n", cmdline, boot_cmd);
	return boot_cmd;
}

int cmdline_lte_mode(char *buf, int str_len)
{
	int offset = str_len;
#ifdef CONFIG_SUPPORT_TDLTE
	offset += sprintf(buf + offset, " ltemode=tcsfb");
#elif defined CONFIG_SUPPORT_WLTE
	offset += sprintf(buf + offset, " ltemode=fcsfb");
#elif defined CONFIG_SUPPORT_LTE
	offset += sprintf(buf + offset, " ltemode=lcsfb");
#endif
	return offset;
}

#else

#ifndef CONFIG_FPGA
int cmdline_fixup_lcd(char *buf, int str_len)
{
	extern uint32_t load_lcd_id_to_kernel();
	uint32_t lcd_id;
	int offset;

	offset = str_len;
	lcd_id = load_lcd_id_to_kernel();
	//add lcd id
	if (lcd_id) {
		offset += sprintf(buf + offset, " lcd_id=ID");
		offset += sprintf(buf + offset, "%x", lcd_id);
	}
	if (lcd_base != NULL) {
		//add lcd frame buffer base, length should be lcd w*h*2(RGB565)
		offset += sprintf(buf + offset, " lcd_base=");
		offset += sprintf(buf + offset, "%x", lcd_base);
	}

	return offset;
}
#endif

#ifdef USB_PHY_TUNE_VALUE
int cmdline_fixup_usb_phy_tune(char *buf, int str_len)
{
	/*transfer this value to kernel usb_hw.c */
	int offset = str_len;

	offset += sprintf(buf + offset, " usb_phy_tune=");
	offset += sprintf(buf + offset, "%x", USB_PHY_TUNE_VALUE);
	//buf[offset] = '\0';

	return offset;
}
#endif
int cmdline_fixup_factorymode(char *buf, int str_len)
{
	int offset = str_len;

	if (1 == is_factorymode()) {
		offset += sprintf(buf + offset, " factory=1");
	}

	return offset;
}

#if defined( CONFIG_AP_ADC_CALIBRATION)||defined(CONFIG_SC8830)||defined(CONFIG_SC9630)||(defined(CONFIG_SC8825) && (!(BOOT_NATIVE_LINUX)))
int cmdline_fixup_adc_data(char *buf, int str_len)
{
	extern int read_adc_calibration_data(char *buffer, int size);
	extern void CHG_SetADCCalTbl(unsigned int *adc_data);
	int offset = str_len;
	unsigned int *adc_data;

	adc_data = malloc(64);
	if (adc_data) {
		memset(adc_data, 0, 64);
		if (0 < read_adc_calibration_data(adc_data, 48)) {
			if (((adc_data[2] & 0xffff) < 4500) && ((adc_data[2] & 0xffff) > 3000) &&
			    ((adc_data[3] & 0xffff) < 4500) && ((adc_data[3] & 0xffff) > 3000)) {
				offset += sprintf(buf + offset, " adc_cal=%d,%d", adc_data[2], adc_data[3]);
			}
			/*just after fgu adc calibration,and no aux adc calibration,need save fgu adc parameters */
			if ((0x00000002 == adc_data[10]) && (0x00000002 & adc_data[11])) {
				offset += sprintf(buf + offset, " fgu_cal=%d,%d,%d", adc_data[4], adc_data[5], adc_data[6]);
			}
#if (defined(CONFIG_SC8825) && (!(BOOT_NATIVE_LINUX)))
			CHG_SetADCCalTbl(adc_data);
			DCDC_Cal_ArmCore();
#endif
		}
		free(adc_data);
	}

	return offset;
}
#endif

int cmdline_fixup_harsh_data(char *buf, int str_len)
{
	int offset = str_len;

	if (0 != read_spldata()) {
		debugf("read_spldata failed\n");
		free(buf);
		return 0;
	}
	if (harsh_data == NULL) {
		debugf("harsh_data malloc failed\n");
		free(buf);
		return 0;
	}
	debugf("spl_data adr 0x%x harsh_data adr 0x%x\n", spl_data, harsh_data);
	if (cal_md5(spl_data, CONFIG_SPL_LOAD_LEN, harsh_data)) {
		offset += sprintf(buf + offset, " securemd5=%08x%08x%08x%08x", *(uint32_t *) harsh_data, *(uint32_t *) (harsh_data + 4),
				  *(uint32_t *) (harsh_data + 8), *(uint32_t *) (harsh_data + 12));
	}
	return offset;
}

int cmdline_fixup_serialno(char *buf, int str_len)
{
	int offset;
	char *sn = get_product_sn();

	offset = str_len;
	offset += sprintf(buf + offset, " androidboot.serialno=%s", sn ? sn : "0123456789ABCDEF");

	return offset;
}

#if defined(CONFIG_SC8830) || (defined CONFIG_SC9630)
int cmdline_fixup_fgu(char *buf, int str_len)
{
	extern unsigned int fgu_cur;
	extern unsigned int fgu_vol;
	int offset;

	offset = str_len;
	offset += sprintf(buf + offset, " fgu_init=%d,%d", fgu_vol, fgu_cur);

	return offset;
}
#endif

int cmdline_fixup_apct_param(char *buf, int str_len)
{
	extern long long lcd_init_time;
	extern long long load_image_time;
	int offset;

	offset = str_len;
	offset += sprintf(buf + offset, " lcd_init=%lld", lcd_init_time);
	offset += sprintf(buf + offset, " load_image=%lld", load_image_time);
	offset += sprintf(buf + offset, " pl_t=%lld", get_ticks());

	return offset;
}
#endif

void cmdline_set_cp_cmdline(char *buf, int str_len)
{
	char *nv_info;
#ifdef CONFIG_SUPPORT_TD
	nv_info = (char *)(((volatile u32 *)CALIBRATION_FLAG));
	sprintf(nv_info, buf);
	nv_info[str_len] = '\0';
	debugf("nv_info:[%08x]%s \n", nv_info, nv_info);
#endif
#ifdef CONFIG_SUPPORT_W
	nv_info = (char *)((volatile u32 *)CALIBRATION_FLAG_WCDMA);
	sprintf(nv_info, buf);
	nv_info[str_len] = '\0';
	debugf("nv_info:[%08x]%s \n", nv_info, nv_info);
#endif
#ifdef CONFIG_SC9630
#ifdef  CONFIG_CP0_ARM0_BOOT
	nv_info = (char *)(((volatile u32 *)CALIBRATION_FLAG_CP0));
	sprintf(nv_info, buf);
	nv_info[str_len] = '\0';
	debugf("nv_info:[%08x]%s \n", nv_info, nv_info);
#endif
	nv_info = (char *)(((volatile u32 *)CALIBRATION_FLAG_CP1));
	sprintf(nv_info, buf);
	nv_info[str_len] = '\0';
	debugf("nv_info:[%08x]%s \n", nv_info, nv_info);
#endif
}

int creat_cmdline(char *cmdline, boot_img_hdr * hdr)
{
	char *buf;
	int offset = 0;
	int ret = 0;

	if (cmdline == NULL) {
		return -1;
	}
	buf = malloc(CMDLINE_BUF_SIZE);
	memset(buf, 0, CMDLINE_BUF_SIZE);

#ifdef CONFIG_OF_LIBFDT
	if (set_boot_mode(cmdline)) {
		if (cmdline && cmdline[0]) {
			offset += sprintf(buf, " %s", cmdline);
		}
#ifdef CONFIG_AP_VERSION
		offset += sprintf(buf + offset, " apv=\"%s\"", CONFIG_AP_VERSION);
#endif
		offset = cmdline_lte_mode(buf, offset);
		cmdline_set_cp_cmdline(buf, offset);
		if (buf != NULL) {
			free(buf);
		}
		return 0;
	} else {
		if (buf != NULL) {
			free(buf);
		}
		return -1;
	}
#else

//	if (hdr) {
//		offset += sprintf(buf, "initrd=0x%x,0x%x", RAMDISK_ADR, hdr->ramdisk_size);
//	}
/* preset loop_per_jiffy */
#ifdef CONFIG_LOOP_PER_JIFFY
	offset += sprintf(buf + offset, " lpj=%d", CONFIG_LOOP_PER_JIFFY);
#endif
#ifdef CONFIG_AP_VERSION
	offset += sprintf(buf + offset, " apv=\"%s\"", CONFIG_AP_VERSION);
#endif
	if (cmdline && cmdline[0]) {
		offset += sprintf(buf + offset, " %s", cmdline);
	}
#ifndef CONFIG_FPGA
	offset = cmdline_fixup_lcd(buf, offset);
#endif
#ifdef USB_PHY_TUNE_VALUE
	offset = cmdline_fixup_usb_phy_tune(buf, offset);
#endif
	offset = cmdline_fixup_factorymode(buf, offset);
	offset += sprintf(buf + offset, " no_console_suspend");
#ifdef CONFIG_RAM_CONSOLE
/* Fill ram log base address and size to cmdline.
It will be used when assigning reserve memory in kernel and dump ram log*/
	offset += sprintf(buf + offset, " boot_ram_log=%#010x,%#x", CONFIG_RAM_CONSOLE_START, CONFIG_RAM_CONSOLE_SIZE);
#endif
	offset = cmdline_fixup_serialno(buf, offset);
	ret = cmdline_fixup_harsh_data(buf, offset);
	if (ret) {
		offset = ret;
	} else {
		return -1;
	}
#if defined(CONFIG_AP_ADC_CALIBRATION)||defined(CONFIG_SC8830) || defined(CONFIG_SC9630) || (defined(CONFIG_SC8825) && (!(BOOT_NATIVE_LINUX)))
	offset = cmdline_fixup_adc_data(buf, offset);
#if defined(CONFIG_SC8830)
	offset = cmdline_fixup_fgu(buf, offset);
#endif
	offset = cmdline_fixup_apct_param(buf, offset);
#endif
	cmdline_set_cp_cmdline(buf, offset);

	debugf("cmdline_len = %d \n pass cmdline: %s \n", strlen(buf), buf);
	creat_atags(VLX_TAG_ADDR, buf, NULL, 0);

	if (buf != NULL) {
		free(buf);
	}
	return 0;
#endif
}

void vlx_entry()
{
	/*down the device if charger disconnect during calibration detect. */
	if (g_charger_mode && !charger_connected()) {
		g_charger_mode = 0;
		power_down_devices();
		while (1) ;
	}
#if !(defined CONFIG_SC8810 || defined CONFIG_TIGER || defined CONFIG_SC8830) || (defined CONFIG_SC9630)
	MMU_InvalideICACHEALL();
#endif

#if (defined CONFIG_SC8810) || (defined CONFIG_SC8825) || (defined CONFIG_SC8830) || (defined CONFIG_SC9630)
	MMU_DisableIDCM();
#endif

#ifdef REBOOT_FUNCTION_INUBOOT
	reboot_func();
#endif

#if BOOT_NATIVE_LINUX
	start_linux();
#else
	void (*entry) (void) = (void *)VMJALUNA_ADR;
	entry();
#endif
}

void normal_mode(void)
{
#if defined (CONFIG_SC8810) || defined (CONFIG_SC8825) || defined (CONFIG_SC8830) || (defined CONFIG_SC9630)
	//MMU_Init(CONFIG_MMU_TABLE_ADDR);
	vibrator_hw_init();
#endif
	set_vibrator(1);

#ifndef UART_CONSOLE_SUPPORT
#ifdef CONFIG_SC7710G2
	extern int serial1_SwitchToModem(void);
	serial1_SwitchToModem();
#endif
#endif

#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS, BACKLIGHT_ON);
#else
	vlx_nand_boot(BOOT_PART, NULL, BACKLIGHT_ON);
#endif

}
void calibration_mode(void)
{
	debugf("calibration_mode\n");
#if defined(BOOT_NATIVE_LINUX_MODEM)
	vlx_nand_boot(RECOVERY_PART,calibration_cmd_buf, BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, calibration_cmd_buf, BACKLIGHT_OFF);
#endif
}
void autotest_mode(void)
{
	debugf("autotest_mode\n");
	vlx_nand_boot(BOOT_PART, calibration_cmd_buf, BACKLIGHT_OFF);
}

void special_mode(void)
{
	debugf("special_mode\n");
#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS " androidboot.mode=special", BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, "androidboot.mode=special", BACKLIGHT_OFF);
#endif

}

void iq_mode(void)
{
	debugf("iq_mode\n");
#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS " androidboot.mode=iq", BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, " androidboot.mode=iq", BACKLIGHT_OFF);
#endif

}

#ifdef CONFIG_GENERIC_MMC
#define MODEM_MEMORY_NAME "modem_memory.log"
#define MODEM_MEMORY_SIZE  (22 * 1024 * 1024)
#ifdef CONFIG_SC8810
#define MODEM_MEMORY_ADDR 0
#elif defined (CONFIG_SC8825) || defined (CONFIG_TIGER) || defined(CONFIG_SC8830) || (defined CONFIG_SC9630)
#define MODEM_MEMORY_ADDR 0x80000000
#endif
#endif
void watchdog_mode(void)
{
	debugf("watchdog_mode\n");
#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS " androidboot.mode=wdgreboot", BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, "androidboot.mode=wdgreboot", BACKLIGHT_OFF);
#endif
}

void unknow_reboot_mode(void)
{
	debugf("unknow_reboot_mode\n");
#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS " androidboot.mode=unknowreboot", BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, "androidboot.mode=unknowreboot", BACKLIGHT_OFF);
#endif
}

void panic_reboot_mode(void)
{
	debugf("%s\n", __func__);
#if BOOT_NATIVE_LINUX
	vlx_nand_boot(BOOT_PART, CONFIG_BOOTARGS " androidboot.mode=panic", BACKLIGHT_OFF);
#else
	vlx_nand_boot(BOOT_PART, "androidboot.mode=panic", BACKLIGHT_OFF);
#endif
}

#if BOOT_NATIVE_LINUX_MODEM

void sipc_addr_reset()
{
#ifdef CONFIG_SC8825
	memset((void *)SIPC_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#elif defined (CONFIG_SC8830)
#if defined(CONFIG_SUPPORT_TD) || defined(CONFIG_SC9620OPENPHONE) || defined(CONFIG_SC9620FPGA)
	memset((void *)SIPC_TD_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);

#elif defined(CONFIG_SUPPORT_W)

	memset((void *)SIPC_WCDMA_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#else
#ifndef CONFIG_NOT_BOOT_TD_MODEM
	memset((void *)SIPC_TD_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#endif
#ifndef CONFIG_NOT_BOOT_W_MODEM
	memset((void *)SIPC_WCDMA_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#endif
#endif
#ifdef CONFIG_SP8830WCN
	memset((void *)SIPC_WCN_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#endif
#endif

#ifdef CONFIG_SC9630
#ifdef CONFIG_CP0_ARM0_BOOT
	memset((void *)SIPC_GGE_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#endif
	memset((void *)SIPC_LTE_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
	memset((void *)SIPC_PMIC_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
#endif

}

#endif
