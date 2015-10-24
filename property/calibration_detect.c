#include "calibration_detect.h"
#include <stdbool.h>

static unsigned int nv_buffer[256]={0};
static int s_is_calibration_mode = 0;
char *calibration_cmd_buf;
uint8_t pctool_cmd_buf[20];
uint8_t pctool_cnf_buf[20];

char *get_calibration_parameter(void)
{
	if(s_is_calibration_mode != 0)
	return calibration_cmd_buf;
	else
	return NULL;
}

bool is_calibration_by_uart(void)
{
       return (2 == s_is_calibration_mode);
}

#define mdelay(_ms) udelay(_ms*1000)
#define CALIBERATE_STRING_LEN 10
#define CALIBERATE_HEAD 0x7e
#define CALIBERATE_COMMOND_T 0xfe
#define CALIBERATE_COMMAND_REQ  1

#define CALIBERATE_DEVICE_NULL  0
#define CALIBERATE_DEVICE_USB   1
#define CALIBERATE_DEVICE_UART  2

extern int charger_connected(void);
typedef  struct tag_cali_command {
	unsigned int   	reserved;
	unsigned short  size;
	unsigned char   cmd;
	unsigned char   sub_cmd;
} COMMAND_T;

extern int serial_tstc(void);
static unsigned long long start_time;
static unsigned long long now_time;



static caliberate_device = CALIBERATE_DEVICE_NULL;

static void send_caliberation_request(void)
{
	COMMAND_T cmd;
        unsigned int i;
        unsigned char *data = (unsigned char *)&cmd;

        cmd.reserved = 0;
        cmd.cmd = CALIBERATE_COMMOND_T;
        cmd.size = CALIBERATE_STRING_LEN-2;
        cmd.sub_cmd = CALIBERATE_COMMAND_REQ;

        serial_putc(CALIBERATE_HEAD);

        for (i = 0; i < sizeof(COMMAND_T); i++)
             serial_putc(data[i]);

        serial_putc(CALIBERATE_HEAD);
}

static int receive_caliberation_response(uint8_t *buf,int len)
{
        int count = 0;
        int ch;
        uint32_t is_not_empty = 0;
        uint32_t start_time = 0,current_time = 0;

	if ((buf == NULL) || (len == 0))
        	return 0;

        is_not_empty = serial_tstc();
        if (is_not_empty) {
             start_time = get_timer_masked();
             do {
                  do {
                  	ch = serial_getc();
                  	if (count < CALIBERATE_STRING_LEN)
		        	buf[count++] = ch;
                  } while (serial_tstc());

                  if ((count >= CALIBERATE_STRING_LEN) || (count >= len)) {
                       caliberate_device = CALIBERATE_DEVICE_UART;
                       break;
                  }

                  current_time = get_timer_masked();
             } while((current_time - start_time) < 500);
        }

        return count;
}

unsigned int check_caliberate(uint8_t * buf, int len)
{
	unsigned int command = 0;
    	unsigned int freq = 0;

	if (len != CALIBERATE_STRING_LEN)
		return 0;

	if ((*buf == CALIBERATE_HEAD) && (*(buf + len -1) == CALIBERATE_HEAD)) {
		if ((*(buf+7) == CALIBERATE_COMMOND_T) && (*(buf + len - 2) != 0x1)) {
			command = *(buf + len - 2);
			command &= 0x7f;

            		freq = *(buf + 1);
            		freq = freq << 8;
            		freq += *(buf + 2);

            		command += freq << 8;
		}
	}

	return command;
}

int pctool_mode_detect_uart(void)
{
	int ret;
	int i ;
	unsigned int caliberate_mode;
	uint8_t buf[20];
	int got = 0;

	printf("%s\n", "uart calibrate detecting");
	loff_t off = 0;
    	send_caliberation_request();

#ifdef CONFIG_MODEM_CALIBERATE
	for(i = 0; i < 20; i++)
		buf[i] = i + 'a';

    	start_time = get_timer_masked();
        printf("uart calibrate configuration start_time=%d\n", start_time);
    	while (1) {
   		got = receive_caliberation_response(buf, sizeof(buf));
   		if (caliberate_device == CALIBERATE_DEVICE_UART)
			break;

    		now_time = get_timer_masked();
		if ((now_time - start_time) > CALIBRATE_ENUM_MS) {
      			printf("usb calibrate configuration timeout\n");
			return -1;
    		}
    	}

	printf("caliberate : what got from host total %d is \n", got);
	for (i = 0; i < got; i++)
		printf("0x%x ", buf[i]);
	printf("\n");

	caliberate_mode = check_caliberate(buf, CALIBERATE_STRING_LEN);
	if (!caliberate_mode) {
		printf("func: %s line: %d caliberate failed\n", __func__, __LINE__);
		return -1;
        } else {
        calibration_cmd_buf=malloc(1024);
        if(calibration_cmd_buf==NULL){
            printf("%s: out of memory\n", __func__);
            return -1;
        }
	memset(calibration_cmd_buf, 0, 1024);
	if (caliberate_device == CALIBERATE_DEVICE_UART)
		sprintf(calibration_cmd_buf, " androidboot.mode=cali calibration=%d,%d,0", caliberate_mode&0xff, (caliberate_mode&(~0xff)) >> 8);
	s_is_calibration_mode = 2;
#if defined(CONFIG_SC7710G2)
	vlx_nand_boot(BOOT_PART, buf, BACKLIGHT_OFF);
#else
	#if defined(BOOT_NATIVE_LINUX_MODEM)

        int str_len = 0;
        char* bootargs = CONFIG_BOOTARGS;
        char* pos = NULL;
        pos = strstr(bootargs, "console=");
        if(NULL != pos)
        {
            str_len = pos-bootargs;
            strncpy(&calibration_cmd_buf[0], bootargs, str_len);
            bootargs = pos;
            pos = strstr(bootargs, " ");
        }
        else
        {
            pos = CONFIG_BOOTARGS;
            str_len = 0;
        }
        sprintf(&calibration_cmd_buf[str_len], "%s", pos);
	str_len = strlen(calibration_cmd_buf);

        if (caliberate_device == CALIBERATE_DEVICE_UART)
            sprintf(&calibration_cmd_buf[str_len], " androidboot.mode=cali calibration=%d,%d,130 ", caliberate_mode&0xff, (caliberate_mode&(~0xff)) >> 8);
        return CMD_CALIBRATION_MODE;
	#else
	return CMD_CALIBRATION_MODE;
	#endif
#endif
	}

    	/* nerver come to here */
   	return -1;
#endif
}

int check_pctool_cmd(uint8_t* buf, int len)
{
    int command = 0;
    unsigned int freq = 0;
    MSG_HEAD_T* msg_head_ptr;
    uint8_t* msg_ptr = buf + 1;
    TOOLS_DIAG_AP_EXIT_CMD_T* msg_exit_prokey;
    TOOLS_DIAG_AP_CMD_T* msg_enter_prokey;

	if((*buf == CALIBERATE_HEAD) && (*(buf + len -1) == CALIBERATE_HEAD)){
	    msg_head_ptr = (MSG_HEAD_T*)(buf + 1);
	    switch(msg_head_ptr->type){
	        case CALIBERATE_COMMOND_T:
				command = msg_head_ptr->subtype;
				command &= 0x3f;
	            freq = *(buf+1);
	            freq = freq<<8;
	            freq += *(buf+2);
	            command += freq<<8;
	            break;
	        case CALIBERATE_PROKEY_COMMOND_T:
	            if(DIAG_AP_CMD_PROGRAM_KEY == *(msg_ptr + sizeof(MSG_HEAD_T))){
	                msg_enter_prokey =  (TOOLS_DIAG_AP_CMD_T*)(msg_ptr + sizeof(MSG_HEAD_T));
	                command = msg_enter_prokey->cmd;
	            }
	            else if(DIAG_AP_CMD_EXIT_PROGRAM_KEY == *(msg_ptr + sizeof(MSG_HEAD_T))){
	                msg_exit_prokey = (TOOLS_DIAG_AP_EXIT_CMD_T*)(msg_ptr + sizeof(MSG_HEAD_T));
	                command = msg_exit_prokey->para;
	            }
			    break;
	        default:
	            command = -1;
	            break;
		}
	}
	printf("checked command from pc , and return value = %d \n" , command);
	return command;
}

extern int power_button_pressed(void);
static int count_ms;
static unsigned long long start_time;
static unsigned long long now_time;

static int recheck_power_button(void)
{
    int cnt = 0;
    int ret = 0;
    do{
        ret = power_button_pressed();
        if(ret == 0)
          cnt++;
        else
          return 1;

        if(cnt>4)
          return 0;
        else{
            mdelay(1);
        }
    }while(1);
}
int is_timeout(void)
{

    now_time = get_timer_masked();

    if(now_time - start_time>count_ms)
      	return 1;
    else{
        return 0;
    }
}

void cali_usb_debug(uint8_t *buf)
{   int ret;
 	int i ;
 	for(i = 0; i<20; i++)
	buf[i] = i+'a';
 	while(!usb_serial_configed)
 		usb_gadget_handle_interrupts();
	printf("USB SERIAL CONFIGED\n");
    gs_open();
#if WRITE_DEBUG
		while(1){
			ret = gs_write(buf, 20);
			printf("func: %s waitting write done\n", __func__);
			if(usb_trans_status)
				printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
			usb_wait_trans_done(1);
			printf("func: %s readly send %d\n", __func__, ret);
		}
#else
		while(1){
			int count = 20;
			usb_wait_trans_done(0);
			if(usb_trans_status)
						printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
			ret = gs_read(buf, &count);
			printf("func: %s readly read %d\n", __func__, count);
			if(usb_trans_status)
				printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
			for(i = 0; i<count; i++)
				printf("%c ", buf[i]);
			printf("\n");
		}

#endif
}
int cali_usb_prepare()
{
	int ret = 0;
	usb_in_cal(1);
	if(dwc_otg_driver_init() < 0)
		{
			printf("%s\n", "dwc_otg_driver_init error");
			return 0;
		}
	if(usb_serial_init() < 0)
		{
			printf("%s\n", "usb_serial_init error");
			return 0;
		}
	return 1;
}
int cali_usb_enum()
{
	int ret = 0;
	count_ms = get_cal_enum_ms();
	start_time = get_timer_masked();
	while(!usb_is_configured()){
		ret = is_timeout();
		if(ret == 0)
			continue;
		else{
			printf("usb calibrate configuration timeout\n");
			return 0;
			}
		}
	printf("USB SERIAL CONFIGED\n");

	start_time = get_timer_masked();
	count_ms = get_cal_io_ms();
	while(!usb_is_port_open()){
		ret = is_timeout();
		if(ret == 0)
			continue;
		else{
			printf("usb calibrate port open timeout\n");
			return 0;
			}
		}
	gs_open();
	printf("USB SERIAL PORT OPENED\n");
	return 1;
}
int cali_get_cmd(uint8_t *buf, int len)
{
	int got = 0;
	int count = len;
	int ret = 0;
	int i;
	start_time = get_timer_masked();

 	while(got < len){
 		if(usb_is_trans_done(0))
		{
			if(usb_trans_status)
				printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
			ret = gs_read(buf + got, &count);
			if(usb_trans_status)
				printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
			for(i=0; i<count; i++)
				dprintf("0x%x \n", buf[got+i]);
			dprintf("\n");
			got+=count;
		}
		if(got<len){
			ret = is_timeout();
			if(ret == 0){
				count=len-got;
				continue;
				}
			else{
				printf("usb read timeout\n");
				return 0;
				}
		}else{
			break;
		}
	}
 	printf("caliberate:what got from host total %d is \n", got);
 	for(i=0; i<got;i++)
 		printf("0x%x ", buf[i]);
 	printf("\n");
	return 1;
}

int reply_to_pctool(uint8_t* buf, int len)
{
	gs_write(buf, len);
	dprintf("func: %s waitting %d write done\n", __func__, len);
	if(usb_trans_status)
		printf("func: %s line %d usb trans with error %d\n", __func__, __LINE__, usb_trans_status);
	usb_wait_trans_done(1);
	start_time = get_timer_masked();
	count_ms = get_cal_io_ms();
	while(!usb_port_open){
		usb_gadget_handle_interrupts();
		if(is_timeout())
			{
			printf("func: %s line %d usb trans timeout", __func__, __LINE__);
			return 0;
 		}
	}
	return 1;
}

int translate_packet(char *dest,char *src,int size)
{
    int i;
    int translated_size = 0;

    dest[translated_size++] = 0x7E;

    for(i=0;i<size;i++){
        if(src[i] == 0x7E){
            dest[translated_size++] = 0x7D;
            dest[translated_size++] = 0x5E;
        } else if(src[i] == 0x7D) {
            dest[translated_size++] = 0x7D;
            dest[translated_size++] = 0x5D;
        } else
            dest[translated_size++] = src[i];
    }
    dest[translated_size++] = 0x7E;
    return translated_size;
}

int prepare_reply_buf(uint8_t* buf, int status)
{
    int ret = 0;
    char *rsp_ptr;
    MSG_HEAD_T* msg_head_ptr;
    TOOLS_DIAG_AP_CNF_T* aprsp;
	int total_len,rsplen;
	printf("preparing reply buf");
    if(NULL == buf){
	    printf("in function prepare_reply_buf, buf = NULL\n");
        return 0;
    }

    msg_head_ptr = (MSG_HEAD_T*)(buf + 1);
    rsplen = sizeof(TOOLS_DIAG_AP_CNF_T) + sizeof(MSG_HEAD_T);
    rsp_ptr = (char*)malloc(rsplen);
    if(NULL == rsp_ptr){
	    printf("in function prepare_reply_buf: Buffer malloc failed\n");
	    return 0;
    }
    aprsp = (TOOLS_DIAG_AP_CNF_T*)(rsp_ptr + sizeof(MSG_HEAD_T));
	msg_head_ptr->len = rsplen;
    memcpy(rsp_ptr,msg_head_ptr,sizeof(MSG_HEAD_T));

    aprsp->status = status;
    aprsp->length = CALIBERATE_CNF_LEN;

    total_len = translate_packet((unsigned char*)pctool_cnf_buf,(unsigned char*)rsp_ptr,((MSG_HEAD_T*)rsp_ptr)->len);
    free(rsp_ptr);
    return total_len;
}

int pctool_mode_detect(void)
{

	int ret , command;
	unsigned int caliberate_mode;
	loff_t off = 0;
        printf("%s\n", "uart cooperating with pc tool");
        if(get_mode_from_gpio())
             return pctool_mode_detect_uart();

        printf("%s\n", "usb cooperating with pc tool");

	if(!charger_connected())
		return -1;

	ret = cali_usb_prepare();
	if (!ret)
		return -1;

#if IO_DEBUG
	cali_usb_debug(pctool_cmd_buf);
#endif
	ret = cali_usb_enum();
	if (!ret)
		return -1;
	ret = cali_get_cmd(pctool_cmd_buf, CALIBERATE_STRING_LEN );
	if (!ret)
		return -1;
	command = check_pctool_cmd(pctool_cmd_buf, CALIBERATE_STRING_LEN);
	printf("func: %s caliberate_mode: %x \n",__func__, command&0xff);
	if (!command)
		{
		printf("func: %s line: %d caliberate failed\n", __func__, __LINE__);
		return -1;
		}
	ret = reply_to_pctool(pctool_cmd_buf, CALIBERATE_STRING_LEN);
	if(!ret)
		return -1;
#ifdef CONFIG_SECURE_BOOT
	if (CALIBERATE_COMMAND_PROGRAMKEY == command)
		{
			ret = cali_get_cmd(pctool_cmd_buf, CALIBERATE_STRING_LEN_14);
			if(!ret)
				return -1;
			command = check_pctool_cmd(pctool_cmd_buf, CALIBERATE_STRING_LEN_14);
			if(!command)
				return -1;
			else if(command = DIAG_AP_CMD_PROGRAM_KEY)
			{
				ret = secure_efuse_program();//jiekou_from_beijing;
			}
            if(!ret)
                ret = prepare_reply_buf(pctool_cmd_buf ,CALIBERATE_CNF_SCS);
            else{
				printk("write efuse failed and error code = %d \n" , ret);
                ret = prepare_reply_buf(pctool_cmd_buf , CALIBERATE_CNF_FAIL);
            }
			if(ret)
				ret = reply_to_pctool(pctool_cnf_buf, ret);
            if(!ret)
	    	    return -1;
			ret = cali_get_cmd(pctool_cmd_buf,CALIBERATE_STRING_LEN_16);
			if(!ret)
				return -1;
            command = check_pctool_cmd(pctool_cmd_buf , CALIBERATE_STRING_LEN_16);
            if(!command)
				return -1;
			switch(command){
				case 0xffff:
					ret = prepare_reply_buf(pctool_cmd_buf ,CALIBERATE_CNF_SCS);
					ret = reply_to_pctool(pctool_cnf_buf, ret);
					return CMD_POWER_DOWN_DEVICE;
					break;
				case 0xfffe:
					ret = prepare_reply_buf(pctool_cmd_buf ,CALIBERATE_CNF_SCS);
					ret = reply_to_pctool(pctool_cnf_buf, ret);
					return CMD_CHARGE_MODE;
					break;
				default:
					ret = prepare_reply_buf(pctool_cmd_buf ,CALIBERATE_CNF_SCS);
					ret = reply_to_pctool(pctool_cnf_buf, ret);
					break;
		    }
        }
#endif
	calibration_cmd_buf=malloc(1024);
       if(calibration_cmd_buf==NULL){
           printf("%s: out of memory\n", __func__);
           return -1;
       }
	memset(calibration_cmd_buf, 0, 1024);
    s_is_calibration_mode=1;
	switch (command & 0xff){
		case CALIBERATE_COMMAND_AUTOTEST:
			sprintf(calibration_cmd_buf, CONFIG_BOOTARGS" androidboot.mode=engtest autotest=1");
			ret = CMD_AUTOTEST_MODE;
			udc_power_off();
			break;
		default:
			sprintf(calibration_cmd_buf,CONFIG_BOOTARGS" androidboot.mode=cali calibration=%d,%d,146 \n", command&0xff, (command&(~0xff))>>8);
			ret = CMD_CALIBRATION_MODE;
			udc_power_off();
			break;
    }
	return ret;
}


int poweron_by_calibration(void)
{
	return s_is_calibration_mode;
}

int cali_file_check(void)
{

#define CALI_MAGIC      (0x49424143) //CALI
#define CALI_COMP       (0x504D4F43) //COMP


	if(do_fs_file_read("prodnv", "/adc.bin", (char *)nv_buffer,sizeof(nv_buffer)))
		return 1;

	if((nv_buffer[0] != CALI_MAGIC)||(nv_buffer[1]!=CALI_COMP))
		return 1;
	else
		return 0;
}

#ifndef CONFIG_AP_ADC_CALIBRATION
#if defined(CONFIG_EMMC_BOOT) && defined (CONFIG_SC8825)
#include "calibration_nv_struct.h"

#define VLX_ADC_ID   2
#define VLX_RAND_TO_U32( _addr )	if( (_addr) & 0x3 ){_addr += 0x4 -((_addr) & 0x3); }

u32 Vlx_GetFixedNvitemAddr(u16 identifier, u32 search_start, u32 search_end)
{
	u32 start_addr, end_addr;
	u16 id, len;
	volatile u16 *flash_ptr;

	start_addr = search_start;
	end_addr   = search_end;
	start_addr += sizeof(u32); //skip update flag

	while(start_addr < end_addr)
	{
		flash_ptr = (volatile u16 *)(start_addr);
		id  = *flash_ptr++;
		len = *flash_ptr;
		if(0xFFFF == id)
		{
			return 0xFFFFFFFF;
		}
		if(identifier == id)
		{
			return (start_addr + 4);
		}
		else
		{
			start_addr += 4 + len +(len & 0x1);
			VLX_RAND_TO_U32( start_addr )
		}
	}
	return 0xFFFFFFFF;
}
#endif

int read_adc_calibration_data(char *buffer,int size)
{
#if  defined (CONFIG_SC8830) || defined (CONFIG_SC9630)
#if 0
	if(do_fs_file_read("prodnv", "/adc.bin", (char *)nv_buffer,sizeof(nv_buffer)))
	    	return 0;
#endif
	if(size>48)
		size=48;
	memcpy(buffer,&nv_buffer[2],size);
	return size;
#elif defined(CONFIG_EMMC_BOOT) && defined (CONFIG_SC8825)
	#define FIXNV_ADR        0x80480000
	calibration_param_T *calibration_base;
	u32 item_base;
	uint16 *value = (uint16 *)(&buffer[8]);
	item_base = Vlx_GetFixedNvitemAddr(VLX_ADC_ID, FIXNV_ADR, (FIXNV_ADR+FIXNV_SIZE));
	if(item_base == 0xFFFFFFFF)
		return 0;
	calibration_base = (calibration_param_T *)item_base;
	if(!((calibration_base->adc).reserved[7] & (BIT_9)))
		return 0;

	value[0] = ((calibration_base->adc).battery[0]) & 0xFFFF;
	value[1] = ((calibration_base->adc).battery[0] >> 16 ) & 0xFFFF;
	value[2] = ((calibration_base->adc).battery[1]) & 0xFFFF;
	value[3] = ((calibration_base->adc).battery[1] >> 16 ) & 0xFFFF;
	return size;
#endif
	return 0;
}
#endif
