
#ifndef _BOOT_IMAGE_H_
#define _BOOT_IMAGE_H_

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8


typedef struct boot_img_hdr boot_img_hdr;

#define ALIGN_SIZE 2048
#define MAX_NAME 512
#define MAGIC_NUM 0x54495A4E

/* DT kernel header */
struct boot_img_hdr {
        unsigned int magic_num;
        unsigned int kernel_addr;
        unsigned int kernel_size;
        unsigned int dt_addr;
        unsigned int dt_size;
        unsigned int tags_addr;
        unsigned int page_size;
        unsigned char cmdline[MAX_NAME];
};


#endif
