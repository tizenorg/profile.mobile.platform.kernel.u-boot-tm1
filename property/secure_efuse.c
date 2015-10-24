/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "efuse_operate.h"
#include "secure_efuse.h"

extern int get_spl_hash(void* hash_data);

int secure_efuse_program(void){

  int ret = 0;
  //printf("secure efuse ------------------------\n");
  ret = calc_sha1_write_efuse();
  if(ret)
    return SHA1_ERR;
  return PROG_OK;
}

int calc_sha1_write_efuse(void){

  union sha_1{
    unsigned char sha1_char[24];
    unsigned int  sha1_int[6];
  }sha1;

  char sha1_string[41];
  int ret = 0;
  memset(&sha1,   0x0, 24);
  memset(sha1_string,   0x0, 41);
//  -----   generate SHA1 key   -----
  get_spl_hash((void*)sha1.sha1_char);
  /*sprintf(sha1_string,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", sha1.sha1_char[0],
				sha1.sha1_char[1],
				sha1.sha1_char[2],
				sha1.sha1_char[3],
				sha1.sha1_char[4],
				sha1.sha1_char[5],
				sha1.sha1_char[6],
				sha1.sha1_char[7],
				sha1.sha1_char[8],
				sha1.sha1_char[9],
				sha1.sha1_char[10],
				sha1.sha1_char[11],
				sha1.sha1_char[12],
				sha1.sha1_char[13],
				sha1.sha1_char[14],
				sha1.sha1_char[15],
				sha1.sha1_char[16],
				sha1.sha1_char[17],
				sha1.sha1_char[18],
				sha1.sha1_char[19]);*/
  sprintf(sha1_string,"%08x%08x%08x%08x%08x", sha1.sha1_int[0],
				sha1.sha1_int[1],
				sha1.sha1_int[2],
				sha1.sha1_int[3],
				sha1.sha1_int[4]);
//  -----   write SHA1 key Efuse   -----
  ret = efuse_hash_write_1(sha1_string, 40);
  if(ret < 0){
    printf("sha1 efuse write Error [%d]\n", ret);
    return SHA1_ERR;
  }
//
  if ( efuse_secure_is_enabled() ){
    printf("enabled\n");
    return PROG_OK;
  }
  efuse_secure_enable();
/*
  char sha2_string[41];
  memset(sha2_string,   0x0, 41);
  efuse_hash_read_1(sha2_string, 40);
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n~~~~~~~[%s]\n~~~~~~~~~~~~~~~~~~~~~~~\n", sha2_string);
*/
  return PROG_OK;
}







