#include <common.h>
#include <malloc.h>
#include <asm/arch/common.h>
#include <asm/arch/sprd_reg.h>
#include <asm/arch/secure_boot.h>
#include <asm/arch/chip_drv_common_io.h>

#define EFUSE_HASH_STARTID 2
/**************************************************************/

#define SHA1CircularShift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))
#define F0_19(A,B,C,D,E,Wt,Kn) \
		E += SHA1CircularShift(5,A) + ((B & C) | ((~B) & D));\
		E += Wt + Kn; \
		B = SHA1CircularShift(30,B);
#define F20_39(A,B,C,D,E,Wt,Kn) \
		E += SHA1CircularShift(5,A) + (B ^ C ^ D);\
		E += Wt + Kn; \
		B = SHA1CircularShift(30,B);
#define F40_59(A,B,C,D,E,Wt,Kn) \
		E += SHA1CircularShift(5,A) + ((B & C) | (B & D) | (C & D));\
		E += Wt + Kn; \
		B = SHA1CircularShift(30,B);
#define F60_79(A,B,C,D,E,Wt,Kn) \
		E += SHA1CircularShift(5,A) + (B ^ C ^ D);\
		E += Wt + Kn; \
		B = SHA1CircularShift(30,B);

PUBLIC int SHA1Reset_32(SHA1Context_32 * context32)
{
	context32->Length_Low = 0;
	context32->Length_High = 0;
	context32->Message_Block_Index = 0;
	context32->Intermediate_Hash[0] = 0x67452301;
	context32->Intermediate_Hash[1] = 0xEFCDAB89;
	context32->Intermediate_Hash[2] = 0x98BADCFE;
	context32->Intermediate_Hash[3] = 0x10325476;
	context32->Intermediate_Hash[4] = 0xC3D2E1F0;
	return 0;
}

int SHA1ProcessMessageBlock_32(SHA1Context_32 * context)
{
	const unsigned int K[] = {	/* Constants defined in SHA-1 */
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	};
	int t;			/* Loop counter */
	unsigned int A, B, C, D, E;	/* Word buffers */
	unsigned int *W;
	unsigned int *H;
	W = context->W;
	H = context->Intermediate_Hash;
	/*
	 * Initialize the first 16 words in the array W
	 */
	for (t = 16; t < 80; t++) {
		context->W[t] = SHA1CircularShift(1, context->W[t - 3] ^ context->W[t - 8] ^ context->W[t - 14] ^ context->W[t - 16]);
	}
	A = H[0];
	B = H[1];
	C = H[2];
	D = H[3];
	E = H[4];

	F0_19(A, B, C, D, E, W[0], K[0])
	    F0_19(E, A, B, C, D, W[1], K[0])
	    F0_19(D, E, A, B, C, W[2], K[0])
	    F0_19(C, D, E, A, B, W[3], K[0])
	    F0_19(B, C, D, E, A, W[4], K[0])
	    F0_19(A, B, C, D, E, W[5], K[0])
	    F0_19(E, A, B, C, D, W[6], K[0])
	    F0_19(D, E, A, B, C, W[7], K[0])
	    F0_19(C, D, E, A, B, W[8], K[0])
	    F0_19(B, C, D, E, A, W[9], K[0])
	    F0_19(A, B, C, D, E, W[10], K[0])
	    F0_19(E, A, B, C, D, W[11], K[0])
	    F0_19(D, E, A, B, C, W[12], K[0])
	    F0_19(C, D, E, A, B, W[13], K[0])
	    F0_19(B, C, D, E, A, W[14], K[0])
	    F0_19(A, B, C, D, E, W[15], K[0])
	    F0_19(E, A, B, C, D, W[16], K[0])
	    F0_19(D, E, A, B, C, W[17], K[0])
	    F0_19(C, D, E, A, B, W[18], K[0])
	    F0_19(B, C, D, E, A, W[19], K[0])

	    F20_39(A, B, C, D, E, W[20], K[1])
	    F20_39(E, A, B, C, D, W[21], K[1])
	    F20_39(D, E, A, B, C, W[22], K[1])
	    F20_39(C, D, E, A, B, W[23], K[1])
	    F20_39(B, C, D, E, A, W[24], K[1])
	    F20_39(A, B, C, D, E, W[25], K[1])
	    F20_39(E, A, B, C, D, W[26], K[1])
	    F20_39(D, E, A, B, C, W[27], K[1])
	    F20_39(C, D, E, A, B, W[28], K[1])
	    F20_39(B, C, D, E, A, W[29], K[1])
	    F20_39(A, B, C, D, E, W[30], K[1])
	    F20_39(E, A, B, C, D, W[31], K[1])
	    F20_39(D, E, A, B, C, W[32], K[1])
	    F20_39(C, D, E, A, B, W[33], K[1])
	    F20_39(B, C, D, E, A, W[34], K[1])
	    F20_39(A, B, C, D, E, W[35], K[1])
	    F20_39(E, A, B, C, D, W[36], K[1])
	    F20_39(D, E, A, B, C, W[37], K[1])
	    F20_39(C, D, E, A, B, W[38], K[1])
	    F20_39(B, C, D, E, A, W[39], K[1])

	    F40_59(A, B, C, D, E, W[40], K[2])
	    F40_59(E, A, B, C, D, W[41], K[2])
	    F40_59(D, E, A, B, C, W[42], K[2])
	    F40_59(C, D, E, A, B, W[43], K[2])
	    F40_59(B, C, D, E, A, W[44], K[2])
	    F40_59(A, B, C, D, E, W[45], K[2])
	    F40_59(E, A, B, C, D, W[46], K[2])
	    F40_59(D, E, A, B, C, W[47], K[2])
	    F40_59(C, D, E, A, B, W[48], K[2])
	    F40_59(B, C, D, E, A, W[49], K[2])
	    F40_59(A, B, C, D, E, W[50], K[2])
	    F40_59(E, A, B, C, D, W[51], K[2])
	    F40_59(D, E, A, B, C, W[52], K[2])
	    F40_59(C, D, E, A, B, W[53], K[2])
	    F40_59(B, C, D, E, A, W[54], K[2])
	    F40_59(A, B, C, D, E, W[55], K[2])
	    F40_59(E, A, B, C, D, W[56], K[2])
	    F40_59(D, E, A, B, C, W[57], K[2])
	    F40_59(C, D, E, A, B, W[58], K[2])
	    F40_59(B, C, D, E, A, W[59], K[2])

	    F60_79(A, B, C, D, E, W[60], K[3])
	    F60_79(E, A, B, C, D, W[61], K[3])
	    F60_79(D, E, A, B, C, W[62], K[3])
	    F60_79(C, D, E, A, B, W[63], K[3])
	    F60_79(B, C, D, E, A, W[64], K[3])
	    F60_79(A, B, C, D, E, W[65], K[3])
	    F60_79(E, A, B, C, D, W[66], K[3])
	    F60_79(D, E, A, B, C, W[67], K[3])
	    F60_79(C, D, E, A, B, W[68], K[3])
	    F60_79(B, C, D, E, A, W[69], K[3])
	    F60_79(A, B, C, D, E, W[70], K[3])
	    F60_79(E, A, B, C, D, W[71], K[3])
	    F60_79(D, E, A, B, C, W[72], K[3])
	    F60_79(C, D, E, A, B, W[73], K[3])
	    F60_79(B, C, D, E, A, W[74], K[3])
	    F60_79(A, B, C, D, E, W[75], K[3])
	    F60_79(E, A, B, C, D, W[76], K[3])
	    F60_79(D, E, A, B, C, W[77], K[3])
	    F60_79(C, D, E, A, B, W[78], K[3])
	    F60_79(B, C, D, E, A, W[79], K[3])
	    H[0] += A;
	H[1] += B;
	H[2] += C;
	H[3] += D;
	H[4] += E;
	return 0;
}

PUBLIC int SHA1Input_32(SHA1Context_32 * context, const unsigned int *message_array, unsigned int length)
{
	while (length--) {
		context->W[context->Message_Block_Index++] = *message_array;
		message_array++;
		context->Length_Low += 32;
		if (context->Length_Low == 0) {
			context->Length_High++;
		}
		if (context->Message_Block_Index == 16) {
			SHA1ProcessMessageBlock_32(context);
			context->Message_Block_Index = 0;
		}
	}
	return 0;
}

int SHA1PadMessage_32(SHA1Context_32 * context)
{
	/*
	 * Check to see if the current message block is too small to hold
	 * the initial padding bits and length. If so, we will pad the
	 * block, process it, and then continue padding into a second
	 * block.
	 */
	unsigned int i, cnt = context->Message_Block_Index;
	context->W[cnt++] = 0x80000000;
	for (i = cnt; i < 16; i++) {
		context->W[i] = 0;
	}

	if (cnt > 14) {
		SHA1ProcessMessageBlock_32(context);
		for (i = 0; i < 14; i++) {
			context->W[i] = 0;
		}
	}
	/*
	 * Store the message length as the last 8 octets
	 */
	context->W[14] = context->Length_High;
	context->W[15] = context->Length_Low;
	SHA1ProcessMessageBlock_32(context);
	return 0;
}

PUBLIC int SHA1Result_32(SHA1Context_32 * context, unsigned char *Message_Digest)
{
	int i;
	uint32_t *ptr;
	ptr = (uint32_t *) context->W;
	SHA1PadMessage_32(context);
	for (i = 0; i < 16; ++i) {
		/* message may be sensitive, clear it out */
		*(ptr + i) = 0;
	}
	context->Length_Low = 0;	/* and clear length */
	context->Length_High = 0;
	for (i = 0; i < SHA1HashSize; ++i) {
		Message_Digest[i] = context->Intermediate_Hash[i >> 2]
		    >> 8 * (3 - (i & 0x03));
	}
	return 0;
}

/******************************************************************/

rom_callback_func_t *get_rom_callback(void)
{
	rom_callback_func_t *rom_callback = NULL;
	rom_callback = (rom_callback_func_t *) (*((unsigned int *)0xFFFF0020));
	return rom_callback;
}

int secureboot_enabled(void)
{
#ifdef CONFIG_SECURE_BOOT
	uint32_t reg = 0;
	uint32_t bonding = REG32(REG_AON_APB_BOND_OPT0);
	if (bonding & BIT_2) {
//              reg = sci_efuse_read(EFUSE_HASH_STARTID);
		reg = __ddie_efuse_read(EFUSE_HASH_STARTID);
		if ((reg >> 31) & 0x1)
			return 1;
	}
#endif
	return 0;
}

#define MAKE_DWORD(a,b,c,d) (uint32_t)(((uint32_t)(a)<<24) | (uint32_t)(b)<<16 | ((uint32_t)(c)<<8) | ((uint32_t)(d)))

void RSA_Decrypt(unsigned char *p, unsigned char *m, unsigned char *r2, unsigned char *e)
{
	rom_callback_func_t *rom_callback = NULL;
	unsigned int _e = 0;
	unsigned int _m[32];
	unsigned int _p[32];
	unsigned int _r2[32];
	int i = 0;

	rom_callback = get_rom_callback();

	_e = MAKE_DWORD(e[0], e[1], e[2], e[3]);

	for (i = 31; i >= 0; i--) {
		_m[31 - i] = MAKE_DWORD(m[4 * i], m[4 * i + 1], m[4 * i + 2], m[4 * i + 3]);
		_p[31 - i] = MAKE_DWORD(p[4 * i], p[4 * i + 1], p[4 * i + 2], p[4 * i + 3]);
		_r2[31 - i] = MAKE_DWORD(r2[4 * i], r2[4 * i + 1], r2[4 * i + 2], r2[4 * i + 3]);
	}

	rom_callback->rsa_modpower(_p, _m, _r2, _e);

	for (i = 31; i >= 0; i--) {
		p[4 * (31 - i)] = (unsigned char)(_p[i] >> 24);
		p[4 * (31 - i) + 1] = (unsigned char)(_p[i] >> 16);
		p[4 * (31 - i) + 2] = (unsigned char)(_p[i] >> 8);
		p[4 * (31 - i) + 3] = (unsigned char)(_p[i]);
	}
}

int harshVerify(uint8_t * data, uint32_t data_len, uint8_t * data_hash, uint8_t * data_key)
{
	uint32_t i, soft_hash_data[32];
	uint32_t *data_ptr;
	vlr_info_t *vlr_info;
	bsc_info_t *bsc_info;
	SHA1Context_32 sha;
	uint8_t hash_copy[128] = { 0 };

	vlr_info = (vlr_info_t *) data_hash;

	if (vlr_info->magic != VLR_MAGIC) {
		printf("harshVerify, vlr magic mismatch\r\n");
		return 0;
	}

	bsc_info = (bsc_info_t *) data_key;
	SHA1Reset_32(&sha);
	SHA1Input_32(&sha, (uint32_t *) data, data_len >> 2);
	SHA1Result_32(&sha, soft_hash_data);
	memcpy(hash_copy, vlr_info->hash, sizeof(vlr_info->hash));

	RSA_Decrypt(hash_copy, bsc_info->key.m, bsc_info->key.r2, (unsigned char *)(&bsc_info->key.e));
	data_ptr = (uint32_t *) (&hash_copy[108]);
	for (i = 0; i < 5; i++) {
		//printf("[%3d] : %02X, %02X . \r\n", i, soft_hash_data[i], data_ptr[i]);
		if (soft_hash_data[i] != data_ptr[i]) {
			printf("harshVerify, mismatch\r\n");
			return 0;
		}
	}
	printf("harshVerify, succ\r\n");
	return 1;
}

void secure_check(uint8_t * data, uint32_t data_len, uint8_t * data_hash, uint8_t * data_key)
{
	if (0 == harshVerify(data, data_len, data_hash, data_key)) {
		while (1) ;
	}
}

void get_sec_callback(sec_callback_func_t * sec_callfunc)
{
	sec_callfunc->rom_callback = get_rom_callback();
	sec_callfunc->secure_check = secure_check;
}

int cal_sha1(void *data, uint32_t orig_len, void *hash_data)
{
	SHA1Context_32 sha;

	SHA1Reset_32(&sha);
	SHA1Input_32(&sha, (uint32_t *) data, orig_len >> 2);
	SHA1Result_32(&sha, hash_data);
	return 1;
}

#ifndef CONFIG_NAND_SPL
int cal_md5(void *data, uint32_t orig_len, void *harsh_data)
{
	return 0;
}
#endif
