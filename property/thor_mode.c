#include <boot_mode.h>
#include <config.h>
#include <common.h>
#include <div64.h>
#include <errno.h>
#include <version.h>
#include <lcd.h>
#include <linux/string.h>
#include "../drivers/mmc/card_sdio.h"
#include "thor_mode.h"
#include "tizen_misc.h"

#ifdef THOR_DEBUG
#define thor_debug(fmt, arg...)		printf(fmt, ## arg)
#else
#define thor_debug(fmt, arg...)
#endif

static unsigned char thor_tx_data_buf[sizeof(struct rsp_box)];
static unsigned char thor_rx_data_buf[sizeof(struct rqt_box)];

static struct rqt_box rqt_buf;
static struct rsp_box rsp_buf;
static struct data_rsp_box data_rsp_buf;

static unsigned int thor_file_size;
static unsigned long long total_file_size;
static unsigned long long downloaded_file_size;

static u32 download_addr = CONFIG_THOR_TRANSFER_BUFFER;

/* partition info */
struct thor_part_info part_info;
static char f_name[F_NAME_BUF_SIZE];

static void thor_clear_part_info(void)
{
	part_info.offset = 0;
	part_info.size = 0;
	part_info.blksz = 0;
	part_info.valid = 0;
	part_info.erase = 0;
}

static void send_rsp(const struct rsp_box *rsp)
{
	memcpy(thor_tx_data_buf, rsp, sizeof(struct rsp_box));
	USB_WriteEx(thor_tx_data_buf, sizeof(struct rsp_box));

	thor_debug("-RSP: %d, %d\n", rsp->rsp, rsp->rsp_data);
}

static void send_data_rsp(signed int ack, signed int count)
{
	struct data_rsp_box *rsp = &data_rsp_buf;

	rsp->ack = ack;
	rsp->count = count;

	memcpy(thor_tx_data_buf, rsp, sizeof(struct data_rsp_box));
	USB_WriteEx(thor_tx_data_buf, sizeof(struct data_rsp_box));

	thor_debug("-DATA RSP: %d, %d\n", ack, count);
}

static int download_file_start(void)
{
	unsigned int dn_addr;
	unsigned int buffered = 0;
	unsigned int remained = thor_file_size;
	unsigned int progressed = 0;
	unsigned int write_ofs = 0;

	int ret;
	int count = 0;
	int download_done = 0;
	int usb_pkt_cnt = 0;
	int per = 0;

	thor_debug("Download file start\n");

	while (!download_done) {
		dn_addr = download_addr + buffered;
		ret = USB_ReadEx((char *) dn_addr, THOR_PACKET_SIZE);
		if (ret <= 0)
			return ret;
		buffered += ret;
		progressed += ret;

		if (progressed >= thor_file_size)
			download_done = 1;

		if (download_done)
			downloaded_file_size += thor_file_size % ret;
		else
			downloaded_file_size += ret;

		per = (int) lldiv(100 * downloaded_file_size, total_file_size);
		draw_progress(per);

		/* MAX UNIT SIZE or done */
		if ((buffered >= THOR_STORE_UNIT_SIZE) || download_done) {
			unsigned int count, remain, mod;

			remain = MIN(buffered, remained);
			mod = remain % part_info.blksz;
			count = remain / part_info.blksz;

			/* write remain byte < block_size */
			if (mod)
				count++;

#ifdef CONFIG_SIG
			/* check board signature when download u-boot-mmc.bin */
			ret = check_board_signature(f_name, download_addr, thor_file_size);
			if (ret) {
				lcd_printf("Signature Check Failed\n");
				return -EINVAL;
			}
#endif
			ret = Emmc_Write(PARTITION_USER, part_info.offset + write_ofs, count, download_addr);
			if (!ret)
				return -EIO;

			write_ofs += count;
			remained -= buffered;
			buffered = 0;
		}

		send_data_rsp(0, ++usb_pkt_cnt);
	}

	return ret;
}

static long long int process_rqt_download(const struct rqt_box *rqt)
{
	struct rsp_box *rsp = &rsp_buf;
	static long long int left, ret_head;
	int file_type, ret = 0;

	memset(rsp, 0, sizeof(struct rsp_box));
	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_DL_INIT:
		thor_file_size = rqt->int_data[0];
		total_file_size = thor_file_size;

		downloaded_file_size = 0;
		/* clear partition info */
		thor_clear_part_info();

		thor_debug("INIT: total %d bytes\n", rqt->int_data[0]);
		break;
	case RQT_DL_FILE_INFO:
		file_type = rqt->int_data[0];
		if (file_type == FILE_TYPE_PIT) {
			thor_debug("PIT table file - not supported\n");
			rsp->ack = -ENOTSUPP;
			ret = rsp->ack;
			break;
		}

		thor_file_size = rqt->int_data[1];
		memcpy(f_name, rqt->str_data[0], F_NAME_BUF_SIZE);

		thor_debug("INFO: name(%s, %d), size(%llu), type(%d)\n",
		      f_name, 0, thor_file_size, file_type);

		rsp->int_data[0] = THOR_PACKET_SIZE;
		/* Get partition info by binary name */
		ret = thor_get_part_info(&part_info, f_name);
		if (ret < 0) {
			thor_debug("Unsupported binary\n");
			rsp->ack = -ENODEV;
			ret = rsp->ack;
		}

		break;
	case RQT_DL_FILE_START:
		send_rsp(rsp);
		return download_file_start();
	case RQT_DL_FILE_END:
		thor_debug("DL FILE_END\n");
		break;
	case RQT_DL_EXIT:
		thor_debug("DL EXIT\n");
		break;
	default:
		thor_debug("Operation not supported: %d", rqt->rqt_data);
		ret = -ENOTSUPP;
	}

	send_rsp(rsp);
	return ret;
}

static int process_rqt_info(const struct rqt_box *rqt)
{
	struct rsp_box *rsp = &rsp_buf;
	memset(rsp, 0, sizeof(struct rsp_box));

	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_INFO_VER_PROTOCOL:
		rsp->int_data[0] = VER_PROTOCOL_MAJOR;
		rsp->int_data[1] = VER_PROTOCOL_MINOR;
		break;
	case RQT_INIT_VER_HW:
		break;
	case RQT_INIT_VER_BOOT:
		sprintf(rsp->str_data[0], "%s", U_BOOT_VERSION);
		break;
	case RQT_INIT_VER_KERNEL:
		sprintf(rsp->str_data[0], "%s", "k unknown");
		break;
	case RQT_INIT_VER_PLATFORM:
		sprintf(rsp->str_data[0], "%s", "p unknown");
		break;
	case RQT_INIT_VER_CSC:
		sprintf(rsp->str_data[0], "%s", "c unknown");
		break;
	default:
		return -EINVAL;
	}

	send_rsp(rsp);
	return 1;
}

static int process_rqt_cmd(const struct rqt_box *rqt)
{
	struct rsp_box *rsp = &rsp_buf;
	memset(rsp, 0, sizeof(struct rsp_box));

	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_CMD_REBOOT:
		thor_debug("TARGET RESET\n");
		send_rsp(rsp);

		udc_power_off();
		thor_save_env("thor");
		reset_cpu(0);

		break;
	case RQT_CMD_POWEROFF:
	case RQT_CMD_EFSCLEAR:
		send_rsp(rsp);
	default:
		thor_debug("Command not supported -> cmd: %d\n", rqt->rqt_data);
		return -EINVAL;
	}

	return 1;
}

static int process_data(void)
{
	struct rqt_box *rqt = &rqt_buf;
	int ret = -EINVAL;

	memcpy(rqt, thor_rx_data_buf, sizeof(struct rqt_box));

	thor_debug("+RQT: %d, %d\n", rqt->rqt, rqt->rqt_data);

	switch (rqt->rqt) {
	case RQT_INFO:
		ret = process_rqt_info(rqt);
		break;
	case RQT_CMD:
		ret = process_rqt_cmd(rqt);
		break;
	case RQT_DL:
		ret = (int) process_rqt_download(rqt);
		break;
	case RQT_UL:
		break;
	default:
		thor_debug("unknown request (%d)", rqt->rqt);
	}

	return ret;
}

int thor_handle(void)
{
	int ret;

retry:
	/* receive the data from Host PC */
	while (1) {
		if (thor_usb_is_connected()) {
			ret = USB_ReadEx(thor_rx_data_buf, strlen("THOR"));
			if (!strncmp(thor_rx_data_buf, "THOR", strlen("THOR")))
			{
				USB_WriteEx("ROHT", strlen("ROHT"));
				goto retry;
			}
			ret = USB_ReadEx(thor_rx_data_buf + strlen("THOR"), sizeof(thor_rx_data_buf) - strlen("THOR"));

			if (ret > 0) {
				ret = process_data();
				if (ret < 0)
					return ret;
			} else {
				thor_debug("%s: No data received!\n", __func__);
				return ret;
			}
		}
	}

	return 0;
}

extern int hw_revision;
void thor_mode(void)
{
	lcd_set_cur_pos(1, 2);
	lcd_printf("THOR MODE\n");

	lcd_set_cur_pos(3, 2);
	lcd_setfgcolor (CONSOLE_COLOR_WHITE);
	lcd_printf("HW REV: %d\n", hw_revision);

	MMU_DisableIDCM();
	thor_USB_Init();

	return;
}
