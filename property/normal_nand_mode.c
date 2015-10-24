#include "normal_mode.h"

#ifdef CONFIG_OF_LIBFDT
#include "dev_tree.h"
#endif

#define KERNL_PAGE_SIZE 2048

#ifdef CONFIG_SUPPORT_TD
static vol_image_required_t s_boot_image_TD_table[]={
	{"tdfixnv1","tdfixnv2",FIXNV_SIZE,TDFIXNV_ADR,IMG_RAW},
	{"tdruntimenv1","tdruntimenv2",RUNTIMENV_SIZE,TDRUNTIMENV_ADR,IMG_RAW},
	{"tdmodem",NULL,TDMODEM_SIZE,TDMODEM_ADR,IMG_RAW},
	{"tddsp",NULL,TDDSP_SIZE,TDDSP_ADR,IMG_RAW},
	{NULL,NULL,0,0,IMG_MAX}
};
#endif

#ifdef CONFIG_SUPPORT_W
static vol_image_required_t s_boot_image_W_table[]={
	{"wfixnv1","wfixnv2",FIXNV_SIZE,WFIXNV_ADR,IMG_RAW},
	{"wruntimenv1","wruntimenv2",RUNTIMENV_SIZE,WRUNTIMENV_ADR,IMG_RAW},
	{"wdsp",NULL,WDSP_SIZE,WDSP_ADR,IMG_RAW},
	{"wmodem",NULL,WMODEM_SIZE,WMODEM_ADR,IMG_RAW},
	{NULL,NULL,0,0,IMG_MAX}
};
#endif

#ifdef CONFIG_SUPPORT_WIFI
static vol_image_required_t s_boot_image_WIFI_table[]={
	{"wcnfixnv1","wcnfixnv2",FIXNV_SIZE,WCNFIXNV_ADR,IMG_RAW},
	{"wcnruntimenv1","wcnruntimenv2",RUNTIMENV_SIZE,WCNRUNTIMENV_ADR,IMG_RAW},
	{"wcnmodem",NULL,WCNMODEM_SIZE,WCNMODEM_ADR,IMG_RAW},
	{NULL,NULL,0,0,IMG_MAX}
};
#endif

static vol_image_required_t* s_boot_image_table[]={
#ifdef CONFIG_SUPPORT_TD
	s_boot_image_TD_table,
#endif

#ifdef CONFIG_SUPPORT_W
	s_boot_image_W_table,
#endif

#ifdef CONFIG_SUPPORT_WIFI
	s_boot_image_WIFI_table,
#endif
	0
};



long long load_image_time = 0;

int read_logoimg(char *bmp_img,size_t size)
{
	//TODO
	return 0;
}

int read_spldata()
{
    struct mtd_device *dev;
    struct part_info *part;
    u8 pnum;
    loff_t off = 0;
    struct mtd_info *nand;
    int size = CONFIG_SPL_LOAD_LEN;

    int ret = find_dev_and_part(SPL_PART, &dev, &pnum, &part);
    if (ret) {
        printf("No partition named %s\n", SPL_PART);
        return -1;
    } else if (dev->id->type != MTD_DEV_TYPE_NAND) {
        printf("Partition %s not a NAND device\n", SPL_PART);
        return -1;
    }
    off = part->offset;
    nand = &nand_info[dev->id->num];

    ret = nand_read_offset_ret(nand, off, &size, (void*)spl_data, &off);
    if(ret != 0) {
        printf("spl nand read error %d\n", ret);
        return -1;
    }
    return 0;
}


int uboot_ubi_read(struct ubi_volume_desc *desc, char *buf,
			   int offset, int size)
{
	int err, lnum, off, len, tbuf_size, i = 0;
	size_t count_save = size;
	void *tbuf;
	unsigned long long tmp;
	struct ubi_volume *vol = NULL;
	struct ubi_device *ubi = NULL;
	loff_t offp = offset;

	vol = desc->vol;
	ubi = desc->vol->ubi;

	printf("read %i bytes from volume %d to %x(buf address)\n",
	       (int) size, vol->vol_id, (unsigned)buf);

	if (vol->updating) {
		printf("updating");
		return -EBUSY;
	}
	if (vol->upd_marker) {
		printf("damaged volume, update marker is set");
		return -EBADF;
	}
	if (offp == vol->used_bytes)
		return 0;

	if (size == 0) {
		printf("Read [%lu] bytes\n", (unsigned long) vol->used_bytes);
		size = vol->used_bytes;
	}

	if (vol->corrupted)
		printf("read from corrupted volume %d", vol->vol_id);
	if (offp + size > vol->used_bytes)
		count_save = size = vol->used_bytes - offp;

	tbuf_size = vol->usable_leb_size;
	if (size < tbuf_size)
		tbuf_size = ALIGN(size, ubi->min_io_size);
	tbuf = malloc(tbuf_size);
	if (!tbuf) {
		printf("NO MEM\n");
		return -ENOMEM;
	}
	len = size > tbuf_size ? tbuf_size : size;

	tmp = offp;
	off = do_div(tmp, vol->usable_leb_size);
	lnum = tmp;
	do {
		if (off + len >= vol->usable_leb_size)
			len = vol->usable_leb_size - off;

		err = ubi_eba_read_leb(ubi, vol, lnum, tbuf, off, len, 0);
		if (err) {
			printf("read err %x\n", err);
			break;
		}
		off += len;
		if (off == vol->usable_leb_size) {
			lnum += 1;
			off -= vol->usable_leb_size;
		}

		size -= len;
		offp += len;

		memcpy(buf, tbuf, len);

		buf += len;
		len = size > tbuf_size ? tbuf_size : size;
	} while (size);

	free(tbuf);
	return err ? err : count_save - size;
}

/**
	attach ubi device to given mtdpart, and return the new
	ubi device num.
*/
static int _boot_ubi_attach_mtd(const char *mtdpart)
{
	struct mtd_device *dev;
	struct part_info *part;
	struct mtd_info *mtd;
	struct mtd_partition mtd_part;
	char mtd_dev[16];
	u8 pnum;
	int ret;

	ret = find_dev_and_part(mtdpart, &dev, &pnum, &part);
	if(ret){
		printf("--->main partition %s miss<---\n",mtdpart);
		return -1;
	}
	if(dev->id->type != MTD_DEV_TYPE_NAND){
		printf("mtd dev %s not a nand device!\n",mtdpart);
		return -1;
	}
	sprintf(mtd_dev, "%s%d", MTD_DEV_TYPE(dev->id->type), dev->id->num);
	mtd = get_mtd_device_nm(mtd_dev);

	memset(&mtd_part, 0, sizeof(mtd_part));
	mtd_part.name = mtdpart;
	mtd_part.size = part->size;
	mtd_part.offset = part->offset;
	add_mtd_partitions(mtd, &mtd_part, 1);
	mtd = get_mtd_device_nm(mtdpart);

	ret = ubi_attach_mtd_dev(mtd, UBI_DEV_NUM_AUTO, 0);
	if(ret<0){
		printf("--->ubi attach mtd %s failed<---\n",mtdpart);
	}
	return ret;
}

/**
	Function for displaying logo.
*/
static void _boot_display_logo(int ubidev, char *part, int backlight_set)
{
	size_t size;
	struct ubi_volume_desc *vol;

#ifdef CONFIG_LCD_720P
	size = 1<<20;
#else
	size = 1<<19;
#endif
	unsigned char * bmp_img = malloc(size);
	if(!bmp_img){
	    printf("%s: malloc for splash image failed!\n",__FUNCTION__);
	    return;
	}
	vol = ubi_open_volume_nm(ubidev, part, UBI_READONLY);
	if (IS_ERR(vol)) {
		printf("cannot open \"%s\", error %d",
			  part, (int)PTR_ERR(vol));
		goto end;
	}

	if(size != uboot_ubi_read(vol, bmp_img, 0, size)) {
		uint32_t nr;
		uint32_t writesize = vol->vol->ubi->mtd->writesize;
		printf("%s: read logo partition failed, retry page by page.\n",__FUNCTION__);
		for(nr = 0; nr < size; nr += writesize) {
			uboot_ubi_read(vol, bmp_img + nr, nr, MIN(writesize,size - nr));
		}
	}

	ubi_close_volume(vol);

	lcd_display_logo(backlight_set,(ulong)bmp_img,size);
end:
	free(bmp_img);
	return;
}

static void _boot_check_and_load_nv(int ubidev, char*vol, char *bakvol, char *addr, int size)
{
	char *buf, *bakbuf;
	char status=0;
	nv_header_t *header;
	int len = size + NV_HEAD_LEN;
	struct ubi_volume_desc *vol_desc;
	struct ubi_volume_desc *bakvol_desc;

	buf = malloc(len);
	if(!buf) {
		printf("%s malloc buf failed.\n",__func__);
		return;
	}
	bakbuf = malloc(len);
	if(!bakbuf) {
		printf("%s malloc bakbuf failed.\n",__func__);
		return;
	}

	vol_desc = ubi_open_volume_nm(ubidev, vol, UBI_READWRITE);
	if (IS_ERR(vol_desc)){
		printf("cannot open \"%s\", error %d\n",vol, (int)PTR_ERR(vol_desc));
		goto bak;
	}
	if(len == uboot_ubi_read(vol_desc, buf, 0, len)) {
		header = (nv_header_t *)buf;
		if(NV_HEAD_MAGIC == header->magic){
			if(_chkNVEcc(buf+NV_HEAD_LEN, header->len,header->checksum))
				status += 1;
		}
	}else {
		printf("%s: ubi vol %s read failed!\n",__func__,vol);
	}
bak:
	bakvol_desc = ubi_open_volume_nm(ubidev, bakvol, UBI_READWRITE);
	if (IS_ERR(bakvol_desc)){
		printf("cannot open \"%s\", error %d\n",bakvol, (int)PTR_ERR(bakvol_desc));
		goto end;
	}
	if(len == uboot_ubi_read(bakvol_desc, bakbuf, 0, len)) {
		header = (nv_header_t *)bakbuf;
		if(NV_HEAD_MAGIC == header->magic){
			if(_chkNVEcc(bakbuf+NV_HEAD_LEN, header->len,header->checksum))
				status += 1<<1;
		}
	}else {
		printf("%s: ubi vol %s read failed!\n",__func__,bakvol);
	}
end:
	if (!IS_ERR(vol_desc))
		ubi_close_volume(vol_desc);
	if (!IS_ERR(vol_desc))
		ubi_close_volume(bakvol_desc);

	switch(status){
		case 0:
			printf("vol %s:both org and bak are damaged!\n",vol);
			memset(addr, 0, size);
			break;
		case 1:
			printf("vol %s:bak is damaged!\n",vol);
			memcpy(addr,buf+NV_HEAD_LEN,size);
			do_raw_data_write(bakvol, len, len, 0, buf);
			break;
		case 2:
			printf("vol %s:org is damaged!\n",vol);
			memcpy(addr,bakbuf+NV_HEAD_LEN,size);
			do_raw_data_write(vol, len, len, 0, bakbuf);
			break;
		case 3:
			printf("vol %s:both org and bak are ok!\n",vol);
			memcpy(addr,buf+NV_HEAD_LEN,size);
			break;
		default:
			printf("%s status error!\n",__func__);
			break;
	}

	free(buf);
	free(bakbuf);

	return;
}

static void _boot_load_required_image(int ubidev, vol_image_required_t info)
{
	struct ubi_volume_desc *vol;
	if(IMG_RAW == info.fs_type){
		if(NULL != info.bak_vol){
			_boot_check_and_load_nv(ubidev, info.vol, info.bak_vol, info.mem_addr, info.size);
		}else{
			vol = ubi_open_volume_nm(ubidev, info.vol, UBI_READWRITE);
			if (IS_ERR(vol)){
				printf("cannot open \"%s\", error %d",
					  info.vol, (int)PTR_ERR(vol));
				return;
			}
			if(info.size != uboot_ubi_read(vol, info.mem_addr, 0, info.size)){
				uint32_t nr;
				uint32_t writesize = vol->vol->ubi->mtd->writesize;
				printf("%s: ubi vol %s read failed,retry page by page.\n",__FUNCTION__,info.vol);
				for(nr = 0; nr < info.size; nr += writesize) {
					uboot_ubi_read(vol, info.mem_addr + nr, nr, MIN(writesize,info.size - nr));
				}
			}
			ubi_close_volume(vol);
		}
	}
	else{
		TODO:
		printf("image type not support now,,,do it later\n");
	}
	return;
}

static int _boot_load_kernel_ramdisk_image(int ubidev, char *part, void *header, void *kaddr, void *rdaddr)
{
	boot_img_hdr *hdr = (boot_img_hdr*)header;
	struct ubi_volume_desc *vol;
	unsigned int size,remain;
	int lnum,offset;
	int ret=0;
	unsigned int dt_img_adr;

	vol = ubi_open_volume_nm(ubidev, part, UBI_READONLY);
	if (IS_ERR(vol)){
		printf("cannot open \"%s\", error %d",
			  part, (int)PTR_ERR(vol));
		return 0;
	}
	
	//image header read and check
	lnum = 0;
	offset = 0;
	size = vol->vol->ubi->mtd->writesize;
	ret = uboot_ubi_read(vol, (char*)hdr, offset, MIN(size,8192));
	if(ret != MIN(size,8192)){
		printf("%s: ubi vol %s read failed,error %d.give up boot!\n",__FUNCTION__,part,ret);
		return 0;
	}
	if(memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)){
		char *n;
		n=hdr->magic;
		printf("bad boot image header, give up boot!!!!\n");
		return 0;
	}
	if(size<hdr->page_size){
		printf("size<hdr->page_size,read hdr again!\n");
		//hdr buf len is 8192, so can't read more than it
		ret = uboot_ubi_read(vol, (char*)hdr, offset, MIN(hdr->page_size,8192));
		if(ret != MIN(hdr->page_size,8192)){
			printf("%s: ubi vol %s read all hdr failed,error %d.give up boot!\n",__FUNCTION__,part,ret);
			return 0;
		}
	}

#ifdef CONFIG_OF_LIBFDT
	//load kernel image to certain addr
	offset = hdr->page_size;
	size = hdr->kernel_size;
	ret = uboot_ubi_read(vol, (char*)kaddr, offset, size);
	if(ret != size){
		printf("%s: kernel image read failed,error %d.give up boot!\n",__FUNCTION__,ret);
		return 0;
	}

	//load radmdisk image
	offset = (offset +size+hdr->page_size-1)&~(hdr->page_size-1);
	size = hdr->ramdisk_size;
	ret = uboot_ubi_read(vol, (char*)rdaddr, offset, size);
	if(ret != size){
		printf("%s: ramdisk image read failed,error %d.give up boot!\n",__FUNCTION__,ret);
		return 0;
	}

	//read dt image
	offset = (offset +size+hdr->page_size-1)&~(hdr->page_size-1);
	size = hdr->dt_size;
	dt_img_adr = RAMDISK_ADR - size - hdr->page_size;
	if(size<0){
		printf("dt size error\n");
		return 0;
	}
	ret = uboot_ubi_read(vol, (char*)dt_img_adr, offset, size);
	if(ret != size)	
	{
		printf("%s:dt read error! offset:0x%x,size:0x%x,dt_img_adr:0x%x\n",__FUNCTION__,offset,size,dt_img_adr);
		return 0;
	}
	if (load_dtb((int)DT_ADR,(void*)dt_img_adr)){
		printf("%s:dt load error!\n",__FUNCTION__);
		return 0;
	}
#else
	//load kernel image to certain addr
	offset = hdr->page_size;
	size = hdr->kernel_size;
	ret = uboot_ubi_read(vol, (char*)kaddr, offset, size);
	if(ret != size){
		printf("%s: kernel image read failed,error %d.give up boot!\n",__FUNCTION__,ret);
		return 0;
	}

	//load radmdisk image
	offset = (offset +size+hdr->page_size-1)&~(hdr->page_size-1);
	size = hdr->ramdisk_size;
	ret = uboot_ubi_read(vol, (char*)rdaddr, offset, size);
	if(ret != size){
		printf("%s: ramdisk image read failed,error %d.give up boot!\n",__FUNCTION__,ret);
		return 0;
	}
#endif
#ifdef CONFIG_SDRAMDISK
	{
	int sd_ramdisk_size = 0;
#ifdef WDSP_ADR
	size = WDSP_ADR - RAMDISK_ADR;
#else
	size = TDDSP_ADR - RAMDISK_ADR;
#endif
	if (size >0)
		sd_ramdisk_size = load_sd_ramdisk((uint8*)RAMDISK_ADR,size);
	if (sd_ramdisk_size >0)
		hdr->ramdisk_size = sd_ramdisk_size;
	}
#endif
	ubi_close_volume(vol);
	return 1;
}

int nand_ubi_dev_init(void)
{
	int ret;
	static int initialized=0;
	static int dev_num=-1;

	if(!initialized){
		//init mtd & ubi devices
		ret = mtdparts_init();
		if(ret){
			printf("mtdparts init error...\n");
			return -1;
		}
		ret = ubi_init();
		if(ret){
			printf("ubi init error...\n");
			return -1;
		}
		//ubi attach mtd dev, and get the new ubi dev num
		dev_num = _boot_ubi_attach_mtd(UBIPAC_PART);
		initialized = 1;
	}

	return dev_num;
}

void vlx_nand_boot(char * kernel_pname, char * cmdline, int backlight_set)
{
	struct ubi_volume_desc *vol;
	boot_img_hdr *hdr = (void *)raw_header;
	int ubi_dev_num;
	int ret;
	int i,j;
	long long start = get_ticks();


	//init mtd ubi module and attach mtd dev
	ubi_dev_num = nand_ubi_dev_init();
	if(ubi_dev_num < 0){
		//dev num can't be a negative
		printf("boot failed...\n");
		return;
	}
	//display logo
#ifdef CONFIG_SPLASH_SCREEN
	_boot_display_logo(ubi_dev_num, LOGO_PART, backlight_set);
	performance_debug("7:");
#endif
	set_vibrator(0);

	//load required image which config in table
	i = 0;
	while(s_boot_image_table[i]){
		j =0;
		while(s_boot_image_table[i][j].vol){
			_boot_load_required_image(ubi_dev_num, s_boot_image_table[i][j]);
			j++;
		}
		i++;
	}
	performance_debug("8:");

	ret = _boot_load_kernel_ramdisk_image(ubi_dev_num, kernel_pname, hdr, KERNEL_ADR, RAMDISK_ADR);
	performance_debug("9:");
	if(!ret){
		printf("boot: load kernel error!\n");
		return;
	}

	load_image_time = get_ticks() - start;
        if(creat_cmdline(cmdline,hdr)){
            debugf("creat_cmdline failed\n");
            return;
        }
	performance_debug("10:");
#if BOOT_NATIVE_LINUX_MODEM
	//sipc addr clear
	sipc_addr_reset();
#endif
	vlx_entry();
	return;
}
