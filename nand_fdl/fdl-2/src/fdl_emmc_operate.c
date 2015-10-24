#include "sci_types.h"
#include "fdl_conf.h"
#ifdef CONFIG_EMMC_BOOT
#include "card_sdio.h"
#include "packet.h"
#include "fdl_crc.h"
#include "fdl_stdio.h"
#include "asm/arch/sci_types.h"
#include <linux/crc32b.h>
#include <malloc.h>
#include <asm/arch/secure_boot.h>
#include "fdl_common.h"
#include "fdl_emmc_operate.h"
#include <ext_common.h>

#ifdef CONFIG_SECURE_BOOT
#include "secure_verify.h"
#endif
#define EFI_SECTOR_SIZE 		(512)
#define EMMC_MAX_MUTIL_WRITE  (0x8000)
#define ERASE_SECTOR_SIZE		((64 * 1024) / EFI_SECTOR_SIZE)
#define EMMC_BUF_SIZE			(((216 * 1024 * 1024) / EFI_SECTOR_SIZE) * EFI_SECTOR_SIZE)

#if defined(CONFIG_SPX30G) || defined(CONFIG_SC9630)
#define SPL_CHECKSUM_LEN        0x8000
#else
#define SPL_CHECKSUM_LEN	0x6000
#endif
#define MAX_GPT_PARTITION_SUPPORT  (50)
#define EMMC_ERASE_ALIGN_LENGTH  (0x800)

static DL_EMMC_FILE_STATUS g_status;
static DL_EMMC_STATUS g_dl_eMMCStatus = {0, 0, 0xffffffff,0, 0, 0,0};

static unsigned long g_checksum;
static unsigned long g_sram_addr;

#if defined (CONFIG_SC8825) || defined (CONFIG_TIGER) || defined (CONFIG_SC8830) ||defined(CONFIG_SC9630)
unsigned char *g_eMMCBuf = (unsigned char*)0x82000000;
#else
unsigned char *g_eMMCBuf = (unsigned char*)0x2000000;
#endif

unsigned int g_total_partition_number = 0;

static wchar_t s_uboot_partition[MAX_UTF_PARTITION_NAME_LEN] = L"uboot";
static wchar_t s_spl_loader_partition[MAX_UTF_PARTITION_NAME_LEN] = L"splloader";

static int uefi_part_info_ok_flag = 0;
static PARTITION_CFG uefi_part_info[MAX_GPT_PARTITION_SUPPORT] = {0};
static PARTITION_CFG s_sprd_emmc_partition_cfg[MAX_GPT_PARTITION_SUPPORT] = {0};

#ifdef CONFIG_SECURE_BOOT
static int check_secure_flag = 0;
static int secure_image_flag = 0;

static wchar_t* const s_force_secure_check[]={
	L"boot",L"recovery",L"uboot",L"tdmodem",L"tddsp",L"wmodem",L"wdsp",L"wcnmodem",L"wl_modem", L"wl_ldsp", L"wl_gdsp", L"wl_warm", L"pm_sys", L"sml",L"tl_gdsp",L"tl_ldsp",L"tl_modem",NULL
};
#endif

/**
	partitions not for raw data or normal usage(e.g. nv and prodinfo) should config here.
	partitions not list here mean raw data/normal usage.
*/
static SPECIAL_PARTITION_CFG const s_special_partition_cfg[]={
	{{L"fixnv1"},{L"fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"runtimenv1"},{L"runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"tdfixnv1"},{L"tdfixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"tdruntimenv1"},{L"tdruntimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"g_fixnv1"},{L"g_fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"g_runtimenv1"},{L"g_runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"l_fixnv1"},{L"l_fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"l_runtimenv1"},{L"l_runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"tl_fixnv1"},{L"tl_fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"tl_runtimenv1"},{L"tl_runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"lf_fixnv1"},{L"lf_fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"lf_runtimenv1"},{L"lf_runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"wfixnv1"},{L"wfixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"wruntimenv1"},{L"wruntimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
        {{L"wl_fixnv1"},{L"wl_fixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
        {{L"wl_runtimenv1"},{L"wl_runtimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"wcnfixnv1"},{L"wcnfixnv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"wcnruntimenv1"},{L"wcnruntimenv2"},IMG_RAW,PARTITION_PURPOSE_NV},
	{{L"system"},NULL,IMG_RAW,PARTITION_PURPOSE_NORMAL},
	{{L"userdata"},NULL,IMG_WITH_SPARSE,PARTITION_PURPOSE_NORMAL},
	{{L"cache"},NULL,IMG_WITH_SPARSE,PARTITION_PURPOSE_NORMAL},
	{{L"prodnv"},NULL,IMG_RAW,PARTITION_PURPOSE_NORMAL},
	{NULL,NULL,IMG_TYPE_MAX,PARTITION_PURPOSE_MAX}
};

static __inline void FDL2_eMMC_SendRep (unsigned long err)
{
	FDL_SendAckPacket (convert_err (err));
}

/**
	just convert partition name wchar to char with violent.
*/
LOCAL __inline char* _w2c(wchar_t *wchar)
{
	static char buf[73]={0};
	unsigned int i=0;
	while((NULL != wchar[i]) && (i<72))
	{
		buf[i] = wchar[i]&0xFF;
		i++;
	}
	buf[i]=0;

	return buf;
}

LOCAL unsigned short eMMCCheckSum(const unsigned int *src, int len)
{
    unsigned int   sum = 0;
    unsigned short *src_short_ptr = PNULL;

    while (len > 3)
    {
        sum += *src++;
        len -= 4;
    }

    src_short_ptr = (unsigned short *) src;

    if (0 != (len&0x2))
    {
        sum += * (src_short_ptr);
        src_short_ptr++;
    }

    if (0 != (len&0x1))
    {
        sum += * ( (unsigned char *) (src_short_ptr));
    }

    sum  = (sum >> 16) + (sum & 0x0FFFF);
    sum += (sum >> 16);

    return (unsigned short) (~sum);
}
LOCAL unsigned int get_pad_data(const unsigned int *src, int len, int offset, unsigned short sum)
{
	unsigned int sum_tmp;
	unsigned int sum1 = 0;
	unsigned int pad_data;
	unsigned int i;
	sum = ~sum;
	sum_tmp = sum & 0xffff;
	sum1 = 0;
	for(i = 0; i < offset; i++) {
		sum1 += src[i];
	}
	for(i = (offset + 1); i < len; i++) {
		sum1 += src[i];
	}
	pad_data = sum_tmp - sum1;
	return pad_data;
}

LOCAL void splFillCheckData(unsigned int * splBuf,  int len)
{
	#define MAGIC_DATA	0xAA55A5A5
	#define CHECKSUM_START_OFFSET	0x28
	#define MAGIC_DATA_SAVE_OFFSET	(0x20/4)
	#define CHECKSUM_SAVE_OFFSET	(0x24/4)

#if defined(CONFIG_TIGER) || defined(CONFIG_SC7710G2) || defined(CONFIG_SC8830) || defined(CONFIG_SC9630)
	EMMC_BootHeader *header;
#if defined(CONFIG_SC8830) || defined(CONFIG_SC9630)
	unsigned int pad_data;
	unsigned int w_len;
	unsigned int w_offset;
	w_len = (SPL_CHECKSUM_LEN-(BOOTLOADER_HEADER_OFFSET+sizeof(*header))) / 4;
	w_offset = w_len - 1;
	//pad the data inorder to make check sum to 0
	pad_data = (unsigned int)get_pad_data((unsigned char*)splBuf+BOOTLOADER_HEADER_OFFSET+sizeof(*header), w_len, w_offset, 0);
	*(volatile unsigned int *)((unsigned char*)splBuf+SPL_CHECKSUM_LEN - 4) = pad_data;
#endif
	header = (EMMC_BootHeader *)((unsigned char*)splBuf+BOOTLOADER_HEADER_OFFSET);
	header->version  = 0;
	header->magicData= MAGIC_DATA;
	header->checkSum = (unsigned int)eMMCCheckSum((unsigned char*)splBuf+BOOTLOADER_HEADER_OFFSET+sizeof(*header), SPL_CHECKSUM_LEN-(BOOTLOADER_HEADER_OFFSET+sizeof(*header)));
#ifdef CONFIG_SECURE_BOOT
	header->hashLen  = CONFIG_SPL_HASH_LEN>>2;
#else
	header->hashLen  = 0;
#endif

#else
	*(splBuf + MAGIC_DATA_SAVE_OFFSET) = MAGIC_DATA;
	*(splBuf + CHECKSUM_SAVE_OFFSET) = (unsigned int)eMMCCheckSum((unsigned int *)&splBuf[CHECKSUM_START_OFFSET/4], SPL_CHECKSUM_LEN - CHECKSUM_START_OFFSET);
//	*(splBuf + CHECKSUM_SAVE_OFFSET) = splCheckSum(splBuf);
#endif
}

LOCAL int _uefi_get_part_info(void)
{
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int i;
	int part_num = 1;

	if(uefi_part_info_ok_flag)
		return 1;

	dev_desc = get_dev("mmc", 1);
	if (dev_desc==NULL) {
		return 0;
	}

	debugf("%s:GPT PARTITION \n",__FUNCTION__);

	if (get_all_partition_info(dev_desc, uefi_part_info, &g_total_partition_number) != 0)
		return 0;

	uefi_part_info_ok_flag = 1;
	return 1;
}

LOCAL unsigned long efi_GetPartBaseSec(wchar_t *partition_name)
{
	int i;

	_uefi_get_part_info();

	for(i=0;i<g_total_partition_number;i++)
	{
		if(wcscmp(partition_name, uefi_part_info[i].partition_name) == 0)
		{
			return uefi_part_info[i].partition_offset;
		}
	}

	//Can't find the specified partition
	debugf("%s: Can't find partition:%s!\n", __FUNCTION__,_w2c(partition_name));
	FDL_SendAckPacket (BSL_INCOMPATIBLE_PARTITION);
	return 0;
}

LOCAL unsigned long efi_GetPartSize(wchar_t *partition_name)
{
	int i;

	_uefi_get_part_info();

	for(i=0;i<g_total_partition_number;i++)
	{
		if(wcscmp(partition_name, uefi_part_info[i].partition_name) == 0)
			return (EFI_SECTOR_SIZE * uefi_part_info[i].partition_size);
	}

	//Can't find the specified partition
	debugf("%s: Can't find partition:%s!\n", __FUNCTION__,_w2c(partition_name));
	FDL_SendAckPacket (BSL_INCOMPATIBLE_PARTITION);
	return 0;
}

LOCAL void _parser_repartition_cfg(unsigned short* partition_cfg, unsigned short total_partition_num)
{
	int i, j;
	unsigned long partition_size;

	/*Decode String: Partition Name(72Byte)+SIZE(4Byte)+...*/
	for(i=0;i<total_partition_num;i++)
	{
		partition_size = *(unsigned long *)(partition_cfg+38*(i+1)-2);
		s_sprd_emmc_partition_cfg[i].partition_index = i+1;
		//the partition size received from tool is MByte.
		if(MAX_SIZE_FLAG == partition_size)
			s_sprd_emmc_partition_cfg[i].partition_size = partition_size;
		else
			s_sprd_emmc_partition_cfg[i].partition_size = 1024*partition_size;
		s_sprd_emmc_partition_cfg[i].partition_attr = PARTITION_RAW;//TODO
		s_sprd_emmc_partition_cfg[i].partition_offset = 0;
		for(j=0;j<MAX_UTF_PARTITION_NAME_LEN;j++)
		{
			s_sprd_emmc_partition_cfg[i].partition_name[j] = *(partition_cfg+38*i+j);
		}

		debugf("%s:partition name:%s,partition_size:0x%x\n",__FUNCTION__,_w2c(s_sprd_emmc_partition_cfg[i].partition_name),s_sprd_emmc_partition_cfg[i].partition_size);
	}
	return;
}

/**
	Get the partition's target image type(raw data or others) and purpose(nv/prodinfo/normal).
*/
LOCAL void _get_partition_attribute(wchar_t* partition_name)
{
	int i;

	for(i=0;s_special_partition_cfg[i].partition != NULL;i++)
	{
		if(wcscmp(s_special_partition_cfg[i].partition, partition_name) == 0)
		{
			g_dl_eMMCStatus.curImgType = s_special_partition_cfg[i].imgattr;
			g_dl_eMMCStatus.partitionpurpose = s_special_partition_cfg[i].purpose;
			debugf("%s:partition %s image type is %d,partitionpurpose:%d\n", __FUNCTION__,_w2c(partition_name),g_dl_eMMCStatus.curImgType,g_dl_eMMCStatus.partitionpurpose);
			return;
		}
	}
	//default type is IMG_RAW
	g_dl_eMMCStatus.curImgType = IMG_RAW;
	g_dl_eMMCStatus.partitionpurpose = PARTITION_PURPOSE_NORMAL;
	debugf("%s:partition %s image type is RAW, normal partition!\n", __FUNCTION__,_w2c(partition_name));
	return;
}

/**
	Check whether the partition to be operated is compatible.
*/
LOCAL BOOLEAN _get_compatible_partition(wchar_t* partition_name)
{
	int i;

	//get special partition attr
	_get_partition_attribute(partition_name);
	//Get user partition info from eMMC
	_uefi_get_part_info();

	//Try to find the partition specified
	if(wcscmp(partition_name, L"uboot") == 0)
	{
		g_dl_eMMCStatus.curUserPartitionName = s_uboot_partition;
		return TRUE;
	}
	if(wcscmp(partition_name, L"splloader") == 0)
	{
		g_dl_eMMCStatus.curUserPartitionName = s_spl_loader_partition;
		return TRUE;
	}

	for(i=0;i<g_total_partition_number;i++)
	{
		if(wcscmp(partition_name, uefi_part_info[i].partition_name) == 0)
		{
			g_dl_eMMCStatus.curUserPartitionName = uefi_part_info[i].partition_name;
			return TRUE;
		}
	}
	//Can't find the specified partition
	g_dl_eMMCStatus.curUserPartitionName = NULL;
	debugf("%s:Can't find partition:%s \n", __FUNCTION__,_w2c(partition_name));
	return FALSE;
}

/**
	Get the backup partition name
*/
LOCAL wchar_t * _get_backup_partition_name(wchar_t *partition_name)
{
	int i = 0;

	for(i=0;s_special_partition_cfg[i].partition != NULL;i++)
	{
		if(wcscmp(partition_name, s_special_partition_cfg[i].partition) == 0)
		{
			return s_special_partition_cfg[i].bak_partition;
		}
	}

	return NULL;
}

/**
	Partition Check Function
	Return:
		0:No partition table or not same with New
		1:Same with new partition
*/
LOCAL int _fdl2_check_partition_table(PARTITION_CFG* new_cfg, PARTITION_CFG* old_cfg, unsigned short total_partition_num)
{
	int i;
	debugf("_fdl2_check_partition_table -----\n");

	uefi_part_info_ok_flag = 0;

	if (!_uefi_get_part_info())
	{
		return 0;
	}

	//we may modify starting LBA of first partition,so we should check it.
	if(STARTING_LBA_OF_FIRST_PARTITION != old_cfg[0].partition_offset)
		return 0;

	if(total_partition_num == g_total_partition_number)
	{
		for(i=0;i<total_partition_num;i++)
		{
			if(0 !=wcscmp(new_cfg[i].partition_name,old_cfg[i].partition_name))
			{
				return 0;
			}
			else
			{
				/*
					the new_cfg partition size get from tool is xx kbyte, 
					the old_cfg partition size read from disk is yy*sector_size byte.
				*/
				if(2*new_cfg[i].partition_size != old_cfg[i].partition_size && new_cfg[i].partition_size !=0xffffffff)
					return 0;
			}
		}
	}
	else{
		return 0;
	}

	return 1;
}

LOCAL int _format_sd_partition(void)
{
	unsigned long part_size = 0;
	unsigned long sd_data_size = 0;
	unsigned long base_sector = 0;

	part_size = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
	if(0 == part_size){
		return -1;
	}
	sd_data_size = newfs_msdos_main(g_eMMCBuf,part_size);
	g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
	base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);

	if (!Emmc_Erase(g_dl_eMMCStatus.curEMMCArea,base_sector,part_size / EFI_SECTOR_SIZE))  {
		return -1;
	}

	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector,sd_data_size / EFI_SECTOR_SIZE, g_eMMCBuf)) {
		return -1;
	}
	return 0;
}

/**
	Erase the whole partition.
*/
LOCAL int _emmc_real_erase_partition(wchar_t *partition_name)
{
	unsigned long count=0, base_sector=0;
	uint8 curArea = 0;

	if(NULL == partition_name)
		return 0;

	if (wcscmp(L"splloader", partition_name) == 0){
		if(secureboot_enabled()){
			return 1;
		}
		count = Emmc_GetCapacity(PARTITION_BOOT1);
		curArea = PARTITION_BOOT1;
		base_sector = 0;
	}else if (wcscmp(L"uboot", partition_name) == 0){
		count = Emmc_GetCapacity(PARTITION_BOOT2);
		curArea = PARTITION_BOOT2;
		base_sector = 0;
	}
	else{
		curArea = PARTITION_USER;
		count = efi_GetPartSize(partition_name); /* partition size : in bytes */
		count = count / EFI_SECTOR_SIZE; /* partition size : in blocks */
		base_sector = efi_GetPartBaseSec(partition_name);
	}

	if(count < EMMC_ERASE_ALIGN_LENGTH)
	{
		unsigned char buf[EMMC_ERASE_ALIGN_LENGTH*EFI_SECTOR_SIZE] = {0xFF};
		if (!Emmc_Write(curArea, base_sector,count,buf))
			return 0;
	}
	else
	{
		if(base_sector%EMMC_ERASE_ALIGN_LENGTH)
		{
			unsigned char buf[EMMC_ERASE_ALIGN_LENGTH*EFI_SECTOR_SIZE] = {0xFF};
			unsigned long base_sector_offset = 0;

			base_sector_offset = EMMC_ERASE_ALIGN_LENGTH - base_sector%EMMC_ERASE_ALIGN_LENGTH;
			if (!Emmc_Write(curArea, base_sector,base_sector_offset,buf))
				return 0;
			count = ((count-base_sector_offset)/EMMC_ERASE_ALIGN_LENGTH)*EMMC_ERASE_ALIGN_LENGTH;
			base_sector = base_sector + base_sector_offset;
		}
		else
			count = (count/EMMC_ERASE_ALIGN_LENGTH)*EMMC_ERASE_ALIGN_LENGTH;

		if(count == 0)
			return 1;

		if (!Emmc_Erase(curArea, base_sector,count))
			return 0;	
	}

	return 1;
}

/**
	Fast erase userpartition use emmc_write .
*/
LOCAL int _emmc_fast_erase_userpartition(wchar_t *partition_name)
{
	unsigned long i, count, len,  base_sector;
	uint8 curArea;

	curArea = PARTITION_USER;
	len = efi_GetPartSize(partition_name);
	count = len / EFI_SECTOR_SIZE;
	base_sector = efi_GetPartBaseSec(partition_name);
	count = (count > ERASE_SECTOR_SIZE)?ERASE_SECTOR_SIZE:count;
	memset(g_eMMCBuf, 0xff, count * EFI_SECTOR_SIZE);		
	if (!Emmc_Write(curArea, base_sector, count, (unsigned char *)g_eMMCBuf))
		 return 0;

	return 1;
}

LOCAL int _emmc_erase_allflash(void)
{
	int i;
	uint32 count;

	_uefi_get_part_info();
	memset(g_eMMCBuf, 0xff, ERASE_SECTOR_SIZE*EFI_SECTOR_SIZE);
	//erase user partitions
	for (i = 0; i < g_total_partition_number; i++) {
		if (!_emmc_fast_erase_userpartition(uefi_part_info[i].partition_name))
			return 0;
	}
	//erase gpt header and partition entry table
	if (!Emmc_Write(PARTITION_USER, 0, ERASE_SECTOR_SIZE, (unsigned char *)g_eMMCBuf))
		 return 0;
	//erase backup gpt header
	count = Emmc_GetCapacity(PARTITION_USER);
	if (!Emmc_Write(PARTITION_USER, count - 1, 1, (unsigned char *)g_eMMCBuf))
		 return 0;
	//erase boot1 partition
	if(!_emmc_real_erase_partition(L"splloader"))
		return 0;
	//erase boot2 partition
	if(!_emmc_real_erase_partition(L"uboot"))
		return 0;

	return 1;
}

#ifdef CONFIG_SECURE_BOOT
int _check_secure_part(wchar_t *partition_name)
{
	int i = 0;
	do
	{
		if(0 == wcscmp(s_force_secure_check[i], partition_name))
			return 1;
		i++;
	}while(s_force_secure_check[i]!=0);

	return 0;
}

LOCAL int _emmc_secure_download(wchar_t* partition_name)
{
	unsigned long count = 0;
	unsigned int saved = 0;
	unsigned int each ;
	unsigned int base_sector = g_dl_eMMCStatus.base_sector;

	//verify the code
	if(secure_image_flag == 0)
		return 1;
	else if(secure_image_flag == 1) {
		if (wcscmp(L"uboot", partition_name) == 0){
			secure_verify(L"splloader",g_eMMCBuf,0);
		}
		else {
			secure_verify(L"fdl2",g_eMMCBuf,0);
		}
	}
	else if(secure_image_flag == 2)
		secure_verify(L"splloader0",g_eMMCBuf,0);

	//download the image
	if (0 == (g_status.unsave_recv_size % EFI_SECTOR_SIZE))
		count = g_status.unsave_recv_size / EFI_SECTOR_SIZE;
	else
		count = g_status.unsave_recv_size / EFI_SECTOR_SIZE + 1;

	//write code
	while(count){
		each = MIN(count,EMMC_MAX_MUTIL_WRITE);
		if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector,
				each, (unsigned char *) (g_eMMCBuf+(saved*EFI_SECTOR_SIZE)))) 
		{
			g_status.unsave_recv_size = 0;
			SEND_ERROR_RSP (BSL_WRITE_ERROR);
			return 0;
		}
		base_sector += each;
		saved += each;
		count -= each;
	}

	//write vlr
	/*
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName)/EFI_SECTOR_SIZE-8,
			8, vlr_buf))
	{
		g_status.unsave_recv_size = 0;
		SEND_ERROR_RSP (BSL_WRITE_ERROR);
		return 0;
	}
	*/

	g_status.unsave_recv_size = 0;
	secure_image_flag = 0;
	return 1;
}
#endif

LOCAL int _emmc_download_image(unsigned long nSectorCount, unsigned long each_write_block, int is_total_recv)
{
	unsigned long cnt, base_sector, trans_times, remain_block;
	unsigned char *point;
	int retval;

#ifdef CONFIG_SECURE_BOOT
	if(secure_image_flag >= 1)
		return 1;
#endif

	if (IMG_WITH_SPARSE == g_dl_eMMCStatus.curImgType){
		retval = write_simg2emmc("mmc", 1, g_dl_eMMCStatus.curUserPartitionName,
			g_eMMCBuf, g_status.unsave_recv_size);
		if (retval == -1) {
			g_status.unsave_recv_size = 0;
			SEND_ERROR_RSP (BSL_WRITE_ERROR);
			return 0;
		}
	} else{
		unsigned long count = nSectorCount;
		unsigned int saved = 0;
		unsigned int each;
		unsigned int base_sector = g_dl_eMMCStatus.base_sector;
		while(count){
			each = MIN(count,each_write_block);
			if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector,
					each, (unsigned char *) (g_eMMCBuf+(saved*EFI_SECTOR_SIZE)))) {
				g_status.unsave_recv_size = 0;
				SEND_ERROR_RSP (BSL_WRITE_ERROR);
				return 0;
			}
			base_sector += each;
			saved += each;
			count -= each;
		}
	}

	if (IMG_WITH_SPARSE == g_dl_eMMCStatus.curImgType){
		if (retval > 0) {
			memmove(g_eMMCBuf, g_eMMCBuf + retval, g_status.unsave_recv_size - retval);
			g_status.unsave_recv_size -= retval;
			g_sram_addr = (unsigned long)(g_eMMCBuf+g_status.unsave_recv_size);
		} else {
			g_status.unsave_recv_size = 0;
			g_sram_addr = (unsigned long)g_eMMCBuf;
		}
	} else {
		g_status.unsave_recv_size = 0;
		if(!is_total_recv){
			g_dl_eMMCStatus.base_sector += nSectorCount;
			g_sram_addr = (unsigned long)g_eMMCBuf;
		}
	}
	return 1;
}

/**
	Function used for reading nv partition which has crc protection.
*/
LOCAL BOOLEAN _read_nv_with_backup(wchar_t *partition_name, uint8* buf, uint32 size)
{
	wchar_t *backup_partition_name = NULL;
	uint8  header_buf[EFI_SECTOR_SIZE];
	u32 base_sector;
	uint16 checkSum = 0;
	uint32 magic = 0,len = 0;
	nv_header_t * header_p = NULL;

	header_p = header_buf;
	base_sector = efi_GetPartBaseSec(partition_name);
	//read origin image header
	memset(header_buf, 0, EFI_SECTOR_SIZE);
	if(!Emmc_Read(PARTITION_USER, base_sector, 1, header_buf)){
		debugf("_read_nv_with_backup read origin image header failed\n");
		return 0;
	}
	memcpy(&magic,header_buf,4);
	if(NV_HEAD_MAGIC == magic){
		base_sector++;
	}
	debugf("_read_nv_with_backup origin image magic = 0x%x\n",magic);
	//------
	//read origin image
	memset(buf, 0xFF, size);
	if(Emmc_Read(PARTITION_USER, base_sector, (size>>9)+1, (uint8*)buf)){
		// get length and checksum
		if(NV_HEAD_MAGIC == magic){
			len = header_p->len;
			checkSum = header_p->checksum;
		}
		else{
			len = size-4;
			checkSum = (uint16)((((uint16)buf[size-3])<<8) | ((uint16)buf[size-4]));
		}
		//check crc
		if(fdl_check_crc(buf, len,checkSum)){
			return 1;
		}
	}

	//----
	//get the backup partition name
	backup_partition_name = _get_backup_partition_name(partition_name);
	if(NULL== backup_partition_name){
		return 0;
	}
	base_sector = efi_GetPartBaseSec(backup_partition_name);

	//read backup header
	memset(header_buf, 0, EFI_SECTOR_SIZE);
	if(!Emmc_Read(PARTITION_USER, base_sector, 1, header_buf)){
		debugf("_read_nv_with_backup read backup image header failed\n");
		return 0;
	}
	memcpy(&magic,header_buf,4);
	if(NV_HEAD_MAGIC == magic){
		base_sector++;
	}
	debugf("_read_nv_with_backup backup image magic = 0x%x\n",magic);

	//read bakup image
	memset(buf, 0xFF, size);
	if(Emmc_Read(PARTITION_USER, base_sector, (size>>9)+1, (uint8*)buf)){
		//get length and checksum
		if(NV_HEAD_MAGIC == magic){
			len = header_p->len;
			checkSum = header_p->checksum;
		}
		else{
			len = size-4;
			checkSum = (uint16)((((uint16)buf[size-3])<<8) | ((uint16)buf[size-4]));
		}
		//check crc
		if(!fdl_check_crc(buf, len,checkSum)){
			debugf("read backup image checksum error \n");;
			return 0;
		}
	}
	return 1;
}

LOCAL int _nv_img_check_and_write(wchar_t* partition, unsigned long size)
{
	unsigned long  nSectorCount, nSectorBase;
	unsigned short sum = 0, *dataaddr;
	wchar_t *backup_partition_name = NULL;
	uint8  header_buf[EFI_SECTOR_SIZE];
	nv_header_t * nv_header_p = NULL;
	//uint32 checksum = 0;

	if (0 == ((size + 4) % EFI_SECTOR_SIZE))
		nSectorCount = (size + 4) / EFI_SECTOR_SIZE;
	else
		nSectorCount = (size + 4) / EFI_SECTOR_SIZE + 1;

	memset(header_buf,0x00,EFI_SECTOR_SIZE);
	nv_header_p = header_buf;
	nv_header_p->magic = NV_HEAD_MAGIC;
	nv_header_p->len = size;
	nv_header_p->checksum = (unsigned long)fdl_calc_checksum(g_eMMCBuf,size);
	nv_header_p->version = NV_VERSION;
	//write the original partition
	_emmc_real_erase_partition(partition);
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector,
	1, (unsigned char *)header_buf)) {
		debugf("%s:original header %s write error! \n", __FUNCTION__,_w2c(partition));
		SEND_ERROR_RSP (BSL_WRITE_ERROR);
		return 0;
	}
	g_dl_eMMCStatus.base_sector++;
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector,
	nSectorCount, (unsigned char *)g_eMMCBuf)) {
		debugf("%s:original %s write error! \n", __FUNCTION__,_w2c(partition));
		SEND_ERROR_RSP (BSL_WRITE_ERROR);
		return 0;
	}

	//write the backup partition
	backup_partition_name = _get_backup_partition_name(partition);
	nSectorBase = efi_GetPartBaseSec(backup_partition_name);
	_emmc_real_erase_partition(backup_partition_name);
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, nSectorBase, 1,
	(unsigned char *)header_buf)) {
		debugf("%s:backup header %s write error! \n", __FUNCTION__,_w2c(partition));
		SEND_ERROR_RSP (BSL_WRITE_ERROR);
		return 0;
	}
	nSectorBase++;
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, nSectorBase, nSectorCount,
	(unsigned char *)g_eMMCBuf)) {
		debugf("%s:backup %s write error! \n", __FUNCTION__,_w2c(partition));
		SEND_ERROR_RSP (BSL_WRITE_ERROR);
		return 0;
	}
	g_status.unsave_recv_size = 0;
	return 1;
}

PUBLIC int fdl2_download_start(wchar_t* partition_name, unsigned long size, unsigned long nv_checksum)
{
	int i = 0;

	debugf("Enter %s,partition:%s,size:0x%x \n", __FUNCTION__,_w2c(partition_name),size);

	g_status.total_size  = size;

#ifdef CONFIG_SECURE_BOOT
	if(_check_secure_part(partition_name))
	{
		check_secure_flag = 1;
		secure_image_flag = 0;
	}else
	{
		check_secure_flag = 0;
		secure_image_flag = 0;
	}
#endif

	if(!_get_compatible_partition(partition_name))
	{
		FDL_SendAckPacket (convert_err (EMMC_INCOMPATIBLE_PART));
		return 0;
	}

	if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose)
	{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
		g_dl_eMMCStatus.part_total_size = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);

		if ((size > g_dl_eMMCStatus.part_total_size) || (size > FIXNV_SIZE)) {
			debugf("%s:size(0x%x) beyond max,size:0x%x,FIXNV_SIZE:0x%x !\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size,FIXNV_SIZE);
			FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
			return 0;
		}
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
		memset(g_eMMCBuf, 0xff, FIXNV_SIZE + EFI_SECTOR_SIZE);
		g_checksum = nv_checksum;
	}
	else if (wcscmp(L"splloader", g_dl_eMMCStatus.curUserPartitionName) == 0)
	{
	#ifdef CONFIG_SECURE_BOOT
		secure_image_flag = 2;
	#endif
		//need to check secure
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT1;
		g_dl_eMMCStatus.part_total_size = EFI_SECTOR_SIZE * Emmc_GetCapacity(PARTITION_BOOT1);
		g_dl_eMMCStatus.base_sector =  0;
		memset(g_eMMCBuf, 0xff, g_dl_eMMCStatus.part_total_size);
		if (size > g_dl_eMMCStatus.part_total_size) {
			debugf("%s:size(0x%x) beyond max size:0x%x!\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size);
			FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
			return 0;
		}
	}
	else if (wcscmp(L"uboot", g_dl_eMMCStatus.curUserPartitionName) == 0)
	{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT2;
		g_dl_eMMCStatus.part_total_size = EFI_SECTOR_SIZE * Emmc_GetCapacity(PARTITION_BOOT2);
		g_dl_eMMCStatus.base_sector =  0;
		memset(g_eMMCBuf, 0xff, g_dl_eMMCStatus.part_total_size);
		if (size > g_dl_eMMCStatus.part_total_size) {
			debugf("%s:size(0x%x) beyond max size:0x%x!\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size);
			FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
			return 0;
		}
	}
	else
	{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
/*
		if (IMG_WITH_SPARSE != g_dl_eMMCStatus.curImgType)
			_emmc_real_erase_partition(g_dl_eMMCStatus.curUserPartitionName);
		else
			memset(g_eMMCBuf, 0, EMMC_BUF_SIZE);
*/
		g_dl_eMMCStatus.part_total_size = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		if (size > g_dl_eMMCStatus.part_total_size) {
			debugf("%s:size(0x%x) beyond max size:0x%x!\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size);
			FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
			return 0;
		}
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	}

	g_status.total_recv_size   = 0;
	g_status.unsave_recv_size   = 0;
	g_sram_addr = (unsigned long)g_eMMCBuf;
	FDL_SendAckPacket (BSL_REP_ACK);
	return 1;
}

PUBLIC int fdl2_download_midst(unsigned short size, char *buf)
{
	unsigned long lastSize, nSectorCount;
	unsigned long each_write_block = EMMC_MAX_MUTIL_WRITE;

	if ((g_status.total_recv_size + size) > g_status.total_size) {
		debugf("%s,size+recvd>total_size!\n", __FUNCTION__);
		FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
		return 0;
	}

	g_status.total_recv_size += size;

#ifdef CONFIG_SECURE_BOOT
	if(check_secure_flag == 1)
	{
		check_secure_flag = 0;
		if(secure_header_parser(buf) != 1)
		{
			secure_image_flag = 0;
			SEND_ERROR_RSP(BSL_WRITE_ERROR);
			return 0;
		}
		else
			secure_image_flag = 1;
	}
#endif

	if (EMMC_BUF_SIZE >= (g_status.unsave_recv_size + size)) 
	{
		memcpy((unsigned char *)g_sram_addr, buf, size);
		g_sram_addr += size;
		g_status.unsave_recv_size += size;

		if (EMMC_BUF_SIZE == g_status.unsave_recv_size)
		{
			if (0 == (EMMC_BUF_SIZE % EFI_SECTOR_SIZE))
				nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE;
			else
				nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE + 1;

			if(!_emmc_download_image(nSectorCount, each_write_block, FALSE))
				return 0;
		}
		else if (g_status.total_recv_size == g_status.total_size) 
		{
			if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose)
			{
				unsigned long  fix_nv_checksum;
				fix_nv_checksum = Get_CheckSum((unsigned char *) g_eMMCBuf, g_status.total_recv_size);
				if (fix_nv_checksum != g_checksum) {
					//may data transfer error
					debugf("%s:nv data transfer error,checksum error!\n", __FUNCTION__);
					SEND_ERROR_RSP(BSL_CHECKSUM_DIFF);
					return 0;
				}
				if(!_nv_img_check_and_write(g_dl_eMMCStatus.curUserPartitionName,FIXNV_SIZE))
					return 0;
			}
			else
			{
				if (g_status.unsave_recv_size != 0) {
					if (0 == (g_status.unsave_recv_size % EFI_SECTOR_SIZE))
						nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE;
					else
						nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE + 1;
					
					if (wcscmp(L"splloader", g_dl_eMMCStatus.curUserPartitionName) == 0){
						if (g_status.total_recv_size < SPL_CHECKSUM_LEN)
							nSectorCount = SPL_CHECKSUM_LEN / EFI_SECTOR_SIZE;
						splFillCheckData((unsigned int *) g_eMMCBuf, (int)g_status.total_recv_size);
					}

					if(!_emmc_download_image(nSectorCount, each_write_block, TRUE))
						return 0;
				}
			}
		} 
	} 
	else
	{
		lastSize = EMMC_BUF_SIZE - g_status.unsave_recv_size;
		memcpy((unsigned char *)g_sram_addr, buf, lastSize);
		g_status.unsave_recv_size = EMMC_BUF_SIZE;
		if (0 == (EMMC_BUF_SIZE % EFI_SECTOR_SIZE))
			nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE;
		else
			nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE + 1;

		if(!_emmc_download_image(nSectorCount, each_write_block, FALSE))
			return 0;

		g_sram_addr = (unsigned long)(g_eMMCBuf + g_status.unsave_recv_size);
		memcpy((unsigned char *)g_sram_addr, (char *)(&buf[lastSize]), size - lastSize);
		g_status.unsave_recv_size += size - lastSize;
		g_sram_addr = (unsigned long)(g_eMMCBuf + g_status.unsave_recv_size);
	 }

	FDL2_eMMC_SendRep (EMMC_SUCCESS);
	return  1; 
}

PUBLIC int fdl2_download_end(void)
{
#ifdef CONFIG_SECURE_BOOT
	if(_emmc_secure_download(g_dl_eMMCStatus.curUserPartitionName) != 1)
	{
		debugf("%s:_emmc_secure_download error!\n", __FUNCTION__);
		FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
		return 0;
	}
#endif
	if(g_status.unsave_recv_size != 0)
	{
		debugf("%s:unsaved size is not zero!\n", __FUNCTION__);
		FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
		return 0;
	}

    	FDL2_eMMC_SendRep (EMMC_SUCCESS);
	g_status.total_size  = 0;	
    	return 1;
}

PUBLIC int fdl2_read_start(wchar_t* partition_name, unsigned long size)
{
	debugf("Enter %s,partition:%s,size:0x%x \n", __FUNCTION__,_w2c(partition_name),size);

	if(!_get_compatible_partition(partition_name))
	{
		FDL_SendAckPacket (convert_err (EMMC_INCOMPATIBLE_PART));
		return FALSE;
	}

	g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;

	if(PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose)
	{
		g_dl_eMMCStatus.part_total_size = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		if ((size > g_dl_eMMCStatus.part_total_size) /*|| (size > FIXNV_SIZE)*/){
			debugf("%s:size(0x%x) beyond max size:0x%x!\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size);
			FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
			return FALSE;
		}
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	}
	else  if (wcscmp(L"splloader", g_dl_eMMCStatus.curUserPartitionName) == 0)
	{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT1;
		g_dl_eMMCStatus.part_total_size = EFI_SECTOR_SIZE * Emmc_GetCapacity(PARTITION_BOOT1);
		g_dl_eMMCStatus.base_sector =  0;
	}
	else if (wcscmp(L"uboot", g_dl_eMMCStatus.curUserPartitionName) == 0)
	{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT2;
		g_dl_eMMCStatus.part_total_size = EFI_SECTOR_SIZE * Emmc_GetCapacity(PARTITION_BOOT2);
		g_dl_eMMCStatus.base_sector =  0;
	}
	else
	{
		g_dl_eMMCStatus.part_total_size = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	}

	if (size > g_dl_eMMCStatus.part_total_size)
	{
		debugf("%s:size(0x%x) beyond max size:0x%x!\n", __FUNCTION__,size,g_dl_eMMCStatus.part_total_size);
		FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
		return FALSE;
	}

	if (wcscmp(L"prodnv", g_dl_eMMCStatus.curUserPartitionName) == 0)
	{
		struct ext2_sblock *sblock=NULL;
		sblock = malloc(MAX(EFI_SECTOR_SIZE,sizeof(struct ext2_sblock)));
		if(!sblock){
			debugf("malloc sblock failed\n");
			goto err;
		}

		if (!Emmc_Read(g_dl_eMMCStatus.curEMMCArea,
				g_dl_eMMCStatus.base_sector + 2,/*superblock offset 2 sect*/
				1,/*superblock take one sect*/
				sblock)){
			free(sblock);
			goto err;
		}

		if(sblock->magic != EXT2_MAGIC){
			debugf("bad prodnv image magic\n");
			free(sblock);
			goto err;
		}
		free(sblock);
	}

	FDL_SendAckPacket (BSL_REP_ACK);
	return TRUE;
err:
	FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
	return FALSE;
}

PUBLIC int fdl2_read_midst(unsigned long size, unsigned long off, unsigned char *buf)
{
	unsigned long nSectorCount, nSectorOffset;

	if ((size +off) > g_dl_eMMCStatus.part_total_size)
	{
		debugf("%s:size(0x%x)+off(0x%x) beyond max size(0x%x)!\n", __FUNCTION__,size,off,g_dl_eMMCStatus.part_total_size);
		FDL2_eMMC_SendRep (EMMC_INVALID_SIZE);
		return FALSE;
	}

	if(PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose)
	{
		if(_read_nv_with_backup(g_dl_eMMCStatus.curUserPartitionName, g_eMMCBuf, FIXNV_SIZE))
		{
			memcpy(buf, (unsigned char *)(g_eMMCBuf + off), size);
			return TRUE;
		}
		else
		{
			FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
			return FALSE;
		}
	}
	else
	{
		if (0 == (size % EFI_SECTOR_SIZE))
			 nSectorCount = size / EFI_SECTOR_SIZE;
		else
			 nSectorCount = size / EFI_SECTOR_SIZE + 1;
		nSectorOffset = off / EFI_SECTOR_SIZE;
		if (!Emmc_Read(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector + nSectorOffset,  nSectorCount, buf ))
		{
			debugf("%s: read error!\n", __FUNCTION__);
			FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
			return FALSE;
		}
	}

	return TRUE;
}
LOCAL void _checkNVPartition(void){
	uint8 *ori_buf;
	uint8 *backup_buf;
	wchar_t *backup_partition_name = NULL;
	uint8  ori_header_buf[EFI_SECTOR_SIZE];
	uint8  backup_header_buf[EFI_SECTOR_SIZE];
	u32 base_sector;
	uint16 checkSum = 0;
	uint32 len = 0;
	nv_header_t * header_p = NULL;
	uint8 status = 0;
	uint32 size = FIXNV_SIZE;

	debugf("check nv partition enter\n");
	ori_buf = malloc(size+EFI_SECTOR_SIZE);
	if(!ori_buf){
		debugf("check nv partition malloc oribuf failed\n");
		return;
	}
	debugf("check nv partition ori_buf 0x%x\n",ori_buf);
	backup_buf = malloc(size+EFI_SECTOR_SIZE);
	if(!backup_buf){
		debugf("check nv partition malloc backup_buf failed\n");
		free(ori_buf);
		return;
	}
	debugf("check nv partition backup_buf 0x%x\n",backup_buf);
	header_p = ori_header_buf;
	base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	//read origin image header
	memset(ori_header_buf, 0, EFI_SECTOR_SIZE);
	if(!Emmc_Read(PARTITION_USER, base_sector, 1, ori_header_buf)){
		debugf("_checkNVPartition read origin image header failed\n");
		free(ori_buf);
		free(backup_buf);
		return;
	}
	if(NV_HEAD_MAGIC == header_p->magic){
		base_sector++;
	}
	debugf("_checkNVPartition origin image magic = 0x%x\n",header_p->magic);
	//------
	//read origin image
	memset(ori_buf, 0xFF, size+EFI_SECTOR_SIZE);
	if(Emmc_Read(PARTITION_USER, base_sector, ((size+EFI_SECTOR_SIZE-1)&~(EFI_SECTOR_SIZE-1))>>9, (uint8*)ori_buf)){
		// get length and checksum
		if(NV_HEAD_MAGIC == header_p->magic){
			len = header_p->len;
			checkSum = header_p->checksum;
		}
		else{
			len = size-4;
			checkSum = (uint16)((((uint16)ori_buf[size-3])<<8) | ((uint16)ori_buf[size-4]));
		}
		//check ecc
		if(fdl_check_crc(ori_buf, len,checkSum)){
			status |= 1;
		}
	}

	//----
	//get the backup partition name
	backup_partition_name = _get_backup_partition_name(g_dl_eMMCStatus.curUserPartitionName);
	if(NULL== backup_partition_name){
		free(ori_buf);
		free(backup_buf);
		return;
	}
	base_sector = efi_GetPartBaseSec(backup_partition_name);
	//read backup header
	header_p = backup_header_buf;
	memset(backup_header_buf, 0, EFI_SECTOR_SIZE);
	if(!Emmc_Read(PARTITION_USER, base_sector, 1, backup_header_buf)){
		debugf("_read_nv_with_backup read backup image header failed\n");
		free(ori_buf);
		free(backup_buf);
		return;
	}

	if(NV_HEAD_MAGIC == header_p->magic){
		base_sector++;
	}
	debugf("_read_nv_with_backup backup image magic = 0x%x\n",header_p->magic);

	//read bakup image
	memset(backup_buf, 0xFF, size+EFI_SECTOR_SIZE);
	if(Emmc_Read(PARTITION_USER, base_sector, ((size+EFI_SECTOR_SIZE-1)&~(EFI_SECTOR_SIZE-1))>>9, (uint8*)backup_buf)){
		//get length and checksum
		if(NV_HEAD_MAGIC == header_p->magic){
			len = header_p->len;
			checkSum = header_p->checksum;
		}
		else{
			len = size-4;
			checkSum = (uint16)((((uint16)backup_buf[size-3])<<8) | ((uint16)backup_buf[size-4]));
		}
		//check ecc
		if(fdl_check_crc(backup_buf, len,checkSum)){
			status |= 2;//four status:00,01,10,11
		}
	}
	switch(status){
		case 0:
			debugf("%s:both org and bak partition are damaged!\n",__FUNCTION__);
			break;
		case 1:
			debugf("%s:bak partition is damaged!\n",__FUNCTION__);
			base_sector = efi_GetPartBaseSec(backup_partition_name);
			header_p = ori_header_buf;
			if(NV_HEAD_MAGIC == header_p->magic){
				if(Emmc_Write(PARTITION_USER, base_sector, 1, ori_header_buf)){
					debugf("write backup nv header success\n");
					base_sector++;
				}
			}
			//write one more sector
			if(Emmc_Write(PARTITION_USER, base_sector, ((size+EFI_SECTOR_SIZE-1)&~(EFI_SECTOR_SIZE-1))>>9, (uint8*)ori_buf)){
				debugf("write backup nv body success\n");
			}
			break;
		case 2:
			debugf("%s:org partition is damaged!\n!",__FUNCTION__);
			base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
			header_p = backup_header_buf;
			if(NV_HEAD_MAGIC == header_p->magic){
				if(Emmc_Write(PARTITION_USER, base_sector, 1, backup_header_buf)){
					debugf("write original partition header success\n");
					base_sector++;
				}
			}
			if(Emmc_Write(PARTITION_USER, base_sector, ((size+EFI_SECTOR_SIZE-1)&~(EFI_SECTOR_SIZE-1))>>9, (uint8*)backup_buf)){
				debugf("write original partition body success\n");
			}
			break;
		case 3:
			debugf("%s:both org and bak partition are ok!\n",__FUNCTION__);
			break;
		default:
			debugf("%s: status error!\n",__FUNCTION__);
			break;
	}
	free(backup_buf);
	free(ori_buf);
	return;
}


PUBLIC int fdl2_read_end(void)
{
	//Just send ack to tool in emmc
	printf("g_dl_eMMCStatus.partitionpurpose = %d\n",g_dl_eMMCStatus.partitionpurpose);
	if(PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose){
		_checkNVPartition();
	}
	FDL_SendAckPacket (BSL_REP_ACK);
	return TRUE;
}

PUBLIC int fdl2_erase(wchar_t* partition_name, unsigned long size)
{
	int retval;
	unsigned long part_size;
	wchar_t *backup_partition_name = NULL;

	if ((wcscmp(partition_name, L"erase_all") == 0) && (size = 0xffffffff)) {
		debugf("%s:Erase all!\n", __FUNCTION__);
		if (!_emmc_erase_allflash()) {
			SEND_ERROR_RSP (BSL_WRITE_ERROR);			
			return 0;
		}
	} else {
		debugf("%s:erase partition %s!\n", __FUNCTION__,_w2c(partition_name));
		if(!_get_compatible_partition(partition_name))
		{
			FDL_SendAckPacket (BSL_INCOMPATIBLE_PARTITION);
			return 0;
		}

		if (!_emmc_real_erase_partition(g_dl_eMMCStatus.curUserPartitionName)) {
			SEND_ERROR_RSP (BSL_WRITE_ERROR);
			return 0;
		}

		backup_partition_name = _get_backup_partition_name(g_dl_eMMCStatus.curUserPartitionName);

		if(NULL != backup_partition_name)
		{
			if (!_emmc_real_erase_partition(backup_partition_name)) {
				SEND_ERROR_RSP (BSL_WRITE_ERROR);
				return 0;
			}
		}

		if (wcscmp(L"sd", g_dl_eMMCStatus.curUserPartitionName) == 0) {
			debugf("formating sd partition, waiting for a while!\n");
			if (_format_sd_partition() == -1){
				debugf("format sd partition failed\n");
				SEND_ERROR_RSP (BSL_WRITE_ERROR);
				return 0;
			}
		}
	}

	FDL2_eMMC_SendRep (EMMC_SUCCESS);
	return 1;
}

PUBLIC int fdl2_repartition(unsigned short* partition_cfg, unsigned short total_partition_num)
{
	int i;

	_parser_repartition_cfg(partition_cfg, total_partition_num);

	if(_fdl2_check_partition_table(s_sprd_emmc_partition_cfg, uefi_part_info, total_partition_num))
	{
		debugf("%s:Partition Config same with before!\n",__FUNCTION__);
		FDL2_eMMC_SendRep (EMMC_SUCCESS);
		return 1;
	}

	for (i = 0; i < 3; i++) {
		write_uefi_partition_table(s_sprd_emmc_partition_cfg);
		if (_fdl2_check_partition_table(s_sprd_emmc_partition_cfg, uefi_part_info, total_partition_num))
			break;
	}

	if (i < 3) {
		FDL2_eMMC_SendRep (EMMC_SUCCESS);
		return 1;
	} else {
		FDL2_eMMC_SendRep (EMMC_SYSTEM_ERROR);
		return 0;
	}
}
#endif

