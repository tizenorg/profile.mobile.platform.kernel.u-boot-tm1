#include <config.h>
#include <common.h>
#include <asm/arch/sci_types.h>
#include "secure_verify.h"

#ifdef  CONFIG_EMMC_BOOT
#else
#include <nand.h>
#endif

#define SHA1_SUM_LEN    20

LOCAL __inline char *_w2c(wchar_t * wchar)
{
	static char buf[73] = { 0 };
	unsigned int i = 0;
	while ((NULL != wchar[i]) && (i < 72)) {
		buf[i] = wchar[i] & 0xFF;
		i++;
	}
	buf[i] = 0;

	return buf;
}

/*
void hexdump(const char *title, const unsigned char *s, int l)
{
    int n=0;

    printf("%s",title);
    for( ; n < l ; ++n)
    {
        if((n%16) == 0)
            printf("\n%04x",n);
        printf(" %02x",s[n]);
    }
    printf("\n");
}
*/

unsigned char sec_header[SEC_HEADER_MAX_SIZE];

int secure_header_parser(uint8_t * header_addr)
{
	if (strcmp(header_addr, HEADER_NAME) == 0)
		return 1;
	else
		return 0;
}

int get_code_addr(wchar_t * name, uint8_t * header_addr)
{
	int addr = 0;
	int i = 0;
	header_info_t *header_p = (header_info_t *) header_addr;

	printf("%s, name: %s enter\r\n", __FUNCTION__, _w2c(name));
	if (name && wcscmp(L"splloader0", name) == 0) {
#ifdef CONFIG_ROM_VERIFY_SPL
		addr = header_addr + CONFIG_BOOTINFO_LENGTH * 2;
#else
		addr = header_addr + CONFIG_SPL_HASH_LEN;
#endif
		return addr;
	}

	for (i = 0; i < header_p->tags_number; i++) {
		if (header_p->tag[i].tag_name == CODE_NAME) {
			addr = header_addr + (header_p->tag[i].tag_offset) * MIN_UNIT;
			break;
		}
	}
#ifdef CONFIG_ROM_VERIFY_SPL
	if (wcscmp(L"splloader", name) == 0) {
		addr -= CONFIG_BOOTINFO_LENGTH;
	}
#endif
	return addr;
}

uint32_t get_code_offset(uint8_t * header_addr)
{
	int i = 0;
	header_info_t *header_p = (header_info_t *) header_addr;

	for (i = 0; i < header_p->tags_number; i++) {
		if (header_p->tag[i].tag_name == CODE_NAME) {
			return header_p->tag[i].tag_offset;
		}
	}
	return 0;
}

int get_vlr_addr(wchar_t * name, uint8_t * header_addr)
{
	int i = 0;
	int addr = 0;
	header_info_t *header_p = (header_info_t *) header_addr;

	printf("%s, name: %s enter\r\n", __FUNCTION__, _w2c(name));
	if (name) {
		if (wcscmp(L"splloader0", name) == 0) {
			addr = header_addr + CONFIG_SPL_LOAD_LEN - VLR_INFO_SIZ;
			return addr;
		}
	}

	for (i = 0; i < header_p->tags_number; i++) {
		if (header_p->tag[i].tag_name == VLR_NAME) {
			addr = header_addr + (header_p->tag[i].tag_offset) * MIN_UNIT;
			break;
		}
	}
	return addr;
}

int get_puk_addr(wchar_t * name, uint8_t * header_addr)
{
	int addr = 0;
	int i = 0;
	int size;
	header_info_t *header_p = (header_info_t *) header_addr;

	printf("%s, name: %s enter\r\n", __FUNCTION__, _w2c(name));
	if (name) {
		if (wcscmp(L"splloader0", name) == 0) {
#ifdef CONFIG_ROM_VERIFY_SPL
			addr = header_addr + CONFIG_BOOTINFO_LENGTH;
#else
			addr = header_addr + CONFIG_SPL_HASH_LEN - KEY_INFO_SIZ;
#endif
			return addr;
		} else if (wcscmp(L"splloader", name) == 0) {
			size = SEC_HEADER_MAX_SIZE;
#ifdef  CONFIG_EMMC_BOOT
			if (TRUE != Emmc_Read(1 /* PARTITION_BOOT1 */ , PUBKEY_BSC_BLOCK_INDEX,
					      PUBKEY_READ_BLOCK_NUMS, (uint8 *) sec_header)) {
				printf("PARTITION_BOOT1 read error \n");
				return 0;
			}
#else
			if (nand_read_skip_bad(&nand_info[0], 0, &size, (uint8 *) sec_header) != 0) {
				printf("PARTITION_BOOT1 read error \n");
				return 0;
			}
#endif
#ifdef CONFIG_ROM_VERIFY_SPL
			addr = sec_header;
#else
			addr = sec_header + CONFIG_SPL_HASH_LEN - KEY_INFO_SIZ;
#endif
			return addr;
		} else if (wcscmp(L"uboot", name) == 0) {
			size = SEC_HEADER_MAX_SIZE;
#ifdef  CONFIG_EMMC_BOOT
			if (TRUE != Emmc_Read(2 /* PARTITION_BOOT2 */ , PUBKEY_VLR_BLOCK_INDEX,
					      PUBKEY_READ_BLOCK_NUMS, (uint8 *) sec_header)) {
				printf("PARTITION_BOOT2 read error \n");
				return 0;
			}
#else
			if (nand_read_skip_bad(&nand_info[0], CONFIG_SYS_NAND_U_BOOT_OFFS, &size, (uint8 *) sec_header) != 0) {
				printf("PARTITION_BOOT2 read error \n");
				return 0;
			}
#endif
#ifdef CONFIG_ROM_VERIFY_SPL
			addr = sec_header;
			return addr;
#endif
			header_p = (header_info_t *) sec_header;
		} else if (wcscmp(L"fdl2", name) == 0) {
			header_p = (header_info_t *) CONFIG_SYS_SDRAM_BASE;
		} else {
			//no operation
		}
	}

	for (i = 0; i < header_p->tags_number; i++) {
		if (header_p->tag[i].tag_name == PUK_NAME) {
			addr = (uint8_t *) header_p + (header_p->tag[i].tag_offset) * MIN_UNIT;
			break;
		}
	}
	return addr;
}

/*copy form libefuse*/
#define UID_BLOCK_START                 0
#define UID_BLOCK_END                   1
#define UID_BLOCK_LEN                  (UID_BLOCK_END - UID_BLOCK_START + 1)
#define HASH_BLOCK_START                2
#define HASH_BLOCK_END                  6
#define HASH_BLOCK_LEN                  (HASH_BLOCK_END - HASH_BLOCK_START + 1)
#define SECURE_BOOT_BLOCK               1
#define SECURE_BOOT_BIT                 18
//#define MIN(a, b)

int HexChar2Dec(unsigned char c)
{
	if ('0' <= c && c <= '9') {
		return (int)(c - '0');
	} else if ('a' <= c && c <= 'f') {
		return ((int)(c - 'a') + 10);
	} else if ('A' <= c && c <= 'F') {
		return ((int)(c - 'A') + 10);
	} else {
		return -1;
	}
}

int str2Num16(const unsigned char *str)
{
	return (str[1] == '\0') ? HexChar2Dec(str[0]) : HexChar2Dec(str[0]) * 16 + HexChar2Dec(str[1]);
}

void convertToLittleEndian(unsigned int *data)
{
	*data = ((*data & 0xff000000) >> 24)
	    | ((*data & 0x00ff0000) >> 8)
	    | ((*data & 0x0000ff00) << 8)
	    | ((*data & 0x000000ff) << 24);
}

int efuse_hash_read(unsigned char *hash, int count)
{
	unsigned char buf[HASH_BLOCK_LEN * 8 + 1] = { 0 };
	unsigned int values[HASH_BLOCK_LEN] = { 0 };
	int i = 0, len = 0, ret = -1;
	unsigned char *ptr = hash;
	int idx;
//      printf("%s()->Line:%d; count = %d \n", __FUNCTION__, __LINE__, count);

	if ((0 == hash) || (count < 1))
		return -1;

	len = MIN(count, sizeof(buf));
	for (i = HASH_BLOCK_START; i <= HASH_BLOCK_END; i++) {
		//ret = efuse_read(i, &values[i - HASH_BLOCK_START]);
//              ret = sci_efuse_read(i);
		ret = __ddie_efuse_read(i);
		values[i - HASH_BLOCK_START] = ret;
		convertToLittleEndian(&values[i - HASH_BLOCK_START]);
		if (ret == 0) {
			printf("efuse read err\n");
			return -2;
		}
	}

	for (i = 0; i < HASH_BLOCK_LEN; i++) {
		sprintf((char *)&buf[i * 8], "%08x", values[i]);
	}

	len = (strlen(buf) % 2 == 0) ? strlen(buf) / 2 : (strlen(buf) / 2 + 1);

	for (i = 0; i < len; ++i) {
		idx = i * 2;
		*ptr++ = str2Num16(buf + idx);
	}

	//strncpy((char *)hash, (char *)&buf, len);
	//printf("%s()->Line:%d; hash = %s, len = %d \n", __FUNCTION__, __LINE__, hash, len-1);
	return (len - 1);
}

void secure_hash_str2ul(void *dst, void *src, unsigned int count)
{
	int i;
	unsigned char tmp[9] = { 0 };
	unsigned char buf[HASH_BLOCK_LEN * 8 + 1] = { 0 };
	unsigned int *pintdst = dst;

	sprintf(buf, "%08x%08x%08x%08x%08x", *(uint32_t *) src, *(uint32_t *) (src + 4), *(uint32_t *) (src + 8), *(uint32_t *) (src + 12),
		*(uint32_t *) (src + 16));
	buf[40] = '\0';
	for (i = 0; i < count; i += 8) {
		memset(tmp, 0, sizeof(tmp));
		strncpy((char *)tmp, (char *)&buf[i], 8);
		tmp[8] = '\0';
		*pintdst++ = (unsigned int)simple_strtoul(tmp, 0, 16);
	}

}

int secure_verify(wchar_t * name, uint8_t * header, uint8_t * code)
{
	int vlr_addr;
	int puk_addr;
	int code_addr;
	unsigned char rom_efuse_hash[SHA1_SUM_LEN] = { 0 };
	unsigned char cal_efuse_hash[SHA1_SUM_LEN] = { 0 };
	unsigned int rom_efuse[HASH_BLOCK_LEN] = { 0 };
	unsigned int cal_efuse[HASH_BLOCK_LEN] = { 0 };
	int i, len;
	NBLHeader *nblheader;
	uint8 *tmpnbl;

	printf("%s, name: %s enter\r\n", __FUNCTION__, _w2c(name));

	tmpnbl = malloc(sizeof(NBLHeader));

	if (name && wcscmp(L"splloader0", name) != 0) {
		if (!secure_header_parser(header))
			while (1) ;
	}

	if (secureboot_enabled()) {
		if (name && (wcscmp(L"splloader0", name) == 0)) {
			/*read efuse hash values */
			efuse_hash_read(rom_efuse_hash, sizeof(rom_efuse_hash));

			/* memset NBLHeader & save NBLHeader */
			nblheader = (NBLHeader *) ((unsigned char *)header + BOOTLOADER_HEADER_OFFSET);
			memcpy(tmpnbl, nblheader, sizeof(NBLHeader));
			len = nblheader->mHashLen;
			/*clear header */
			memset(nblheader, 0, sizeof(NBLHeader));
			nblheader->mHashLen = len;
			cal_sha1(header, (nblheader->mHashLen) << 2, cal_efuse_hash);

			/*restore the NBLHeader */
			memcpy(nblheader, tmpnbl, sizeof(NBLHeader));

			/*convert char to int */
			secure_hash_str2ul(&cal_efuse[0], cal_efuse_hash, 2 * SHA1_SUM_LEN);
			secure_hash_str2ul(&rom_efuse[0], rom_efuse_hash, 2 * SHA1_SUM_LEN);

			for (i = 0; i < 5; i++) {
				rom_efuse[i] &= (~0x80000000);	//BIT31
				cal_efuse[i] &= (~0x80000000);	//BIT31
			}
			//hexdump ("setrom_efuse:", rom_efuse, sizeof (rom_efuse));
			//hexdump ("setpp:", cal_efuse, sizeof (cal_efuse));

			if ((rom_efuse[0] == cal_efuse[0]) && (rom_efuse[1] == cal_efuse[1]) && (rom_efuse[2] == cal_efuse[2]) && (rom_efuse[3] == cal_efuse[3])
			    && (rom_efuse[4] == cal_efuse[4])) {
				printf("efuse hash compare successful\n");
			} else {
				printf("efuse hash compare difference\n");
				while (1) ;
			}
		}
	}

	puk_addr = get_puk_addr(name, header);
//    hexdump("puk_addr:", puk_addr, 260);

	vlr_addr = get_vlr_addr(name, header);

	if (code) {
		code_addr = code;
	} else {
		code_addr = get_code_addr(name, header);
	}
	printf("puk_addr=%x,vlr_addr=%x,code_addr=%x\n", puk_addr, vlr_addr, code_addr);

	secure_check((uint8_t *) code_addr, ((vlr_info_t *) vlr_addr)->length, (uint8_t *) vlr_addr, (uint8_t *) puk_addr);
}
