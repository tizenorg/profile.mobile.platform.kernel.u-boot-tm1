/******************************************************************************
 ** File Name:      DRV_usb.c                                                 *
 ** Author:         JiaYong.Yang                                              *
 ** DATE:           09/01/2010                                                *
 ** Copyright:      2010 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************/
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include <config.h>
#include <common.h>

#include <asm/arch/common.h>
#include <asm/arch/usb200_fdl.h>
#include <asm/arch/drv_usb20.h>
#include <asm/arch/virtual_com.h>
#include <asm/arch/packet.h>
#include <asm/arch/usb20_reg_v3.h>

#define USB_DEBUG

#ifdef USB_DEBUG
#define usb_debug(fmt, arg...)		printf(fmt, ## arg)
#else
#define usb_debug(fmt, arg...)
#endif

typedef enum
{
	USB_HIGH, USB_FULL, USB_LOW
} USB_SPEED;

#define USB_DEVICE_QUALIFIER_TYPE                 0x06
#define USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE                 0x07


#define PUBLIC
#define LOCAL static
int rdx=0;
int x=0;
int error=0;
int length;
int check_usb_reconnected = 0;

extern void usb_power_on(void);

extern void Dcache_InvalRegion(unsigned int addr, unsigned int length);
extern void Dcache_CleanRegion(unsigned int addr, unsigned int length);
extern int panic_display(void);

static __inline void usb_handler (void);

//static int booting_mode = BOOT_MODE_NORMAL;

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#define BULK_MPS		USB_PACKET_512
#define EP_OUT			USB_EP2
#define EP_IN			USB_EP1
#define EP_INTERRUPT		USB_EP3
#define MAX_RECV_LENGTH         (BULK_MPS) 
#define MAX_PACKET_SIZE			(BULK_MPS)
#define USB_TIMEOUT             (1000)

#define USB_SUSPEND_AFTER_USB_MODE_START    0 // step 2. if (usb suspend occurred) after usb mode started
#define USB_STARTED                         1 // step 1. after usb mode start this value will set
#define USB_DETACHED_AFTER_USB_MODE_START   2 // step 3. if (usb suspend occurred) and (musb cable detached) -> goto usb restart
#define USB_BEFORE_USB_MODE                 4 // step 0. default value before usb mode start

/**---------------------------------------------------------------------------*
 **                         Data Structures                                   *
 **---------------------------------------------------------------------------*/
__align(32) uint8 line_coding_config[7] = {0x00,0xc2,0x01,0x00,0x00,0x00,0x08};
__align (32) LOCAL uint32    s_setup_packet[8] = {0};
__align (32) LOCAL unsigned char usb_out_endpoint_buf[2] [MAX_RECV_LENGTH];
/*lint -e551 for "enum_speed" */
uint32    enum_speed = 0;
LOCAL uint32    recv_length = 0;
static unsigned int g_cdc_status = 0;
LOCAL uint32    s_comm_feature = 0;
/*++for ch9 test*/
uint32  g_cdc_state = 0;
LOCAL uint32 g_cdc_configuration = 1;
LOCAL uint32 g_cdc_interface = 1;
/*--for ch9 test*/
LOCAL uint32 g_usb_status = USB_BEFORE_USB_MODE;
/*****************************************************************************/
//  Description:   configure out endpoint0 to receive setup message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL int EPO0_config (BOOLEAN is_dma, uint32 *buffer)
{
	// Programs DOEPTSIZ0 register with Packet Count and Transfer Size
	* (volatile uint32 *) USB_DOEP0TSIZ |= (unsigned int) (BIT_29|BIT_30); //setup packet count , 3 packet
	* (volatile uint32 *) USB_DOEP0TSIZ |= (unsigned int) (BIT_3|BIT_4); //set Transfer Size ,24 bytes

	if (is_dma)
	{
		* (volatile uint32 *) USB_DOEPDMA (0) = (uint32) buffer;//lint !e718
	}

	* (volatile uint32 *) USB_DOEP0CTL |= (unsigned int) BIT_26;    // set clear NAK
	* (volatile uint32 *) USB_DOEP0CTL |= (unsigned int) BIT_31;    // set endpoint enable
	return 0;
}
/*****************************************************************************/
//  Description:   configure in endpoint0 to send message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void EPI0_config (uint32 transfer_size, uint32 packet_count, BOOLEAN is_dma, uint32 *buffer)
{
	volatile USB_DIEP0TSIZ_U *diep0tsiz_ptr = (USB_DIEP0TSIZ_U *) USB_DIEP0TSIZ;

	diep0tsiz_ptr->mBits.transfer_size = transfer_size;
	diep0tsiz_ptr->mBits.packet_count = packet_count;

	if (is_dma)
	{
		Dcache_CleanRegion((unsigned int)(buffer), transfer_size);
		* (volatile uint32 *) USB_DIEPDMA (0) = (uint32) buffer;//lint !e718
	}

	* (volatile uint32 *) USB_DIEP0CTL &= (unsigned int) (~ (BIT_22|BIT_23|BIT_24|BIT_25)); // set EP0 in tx fifo nummber

	* (volatile uint32 *) USB_DIEP0CTL |= (unsigned int) BIT_26;                    // clear NAK

	* (volatile uint32 *) USB_DIEP0CTL |= (unsigned int) BIT_31;                    // set endpoint enable

	while(1)
	{
		diep0tsiz_ptr = (USB_DIEP0TSIZ_U *) USB_DIEP0TSIZ;
		if(diep0tsiz_ptr->dwValue==0)
		{
			break;
		}
	}
	diep0tsiz_ptr->dwValue=0;
}
/*****************************************************************************/
//  Description:   usb reset interrupt handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_EPActive (USB_EP_NUM_E ep_num, BOOLEAN dir)
{
	if (dir)	// out endpoint
	{
		*(volatile uint32 *) USB_DOEPCTL (ep_num) |= (unsigned int) BIT_15;	//lint !e718// endpoint active
		* (volatile uint32 *) USB_DOEPMSK = (unsigned int) (BIT_13|BIT_12|BIT_3|BIT_0);
	}
	else
	{
		*(volatile uint32 *) USB_DIEPCTL (ep_num) |= (unsigned int) BIT_15;	//lint !e718// endpoint active
		*(volatile uint32 *) USB_DIEPMSK = 0xffffffff;
	}
}
/*****************************************************************************/
//  Description:   configure specified endpoint to send/receive message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void EPn_config (USB_EP_NUM_E ep_num, USB_EP_TYPE_E ep_type, BOOLEAN dir, uint32 mps)
{
	// out endpoint
	if (dir)
	{
		volatile USB_DOEPCTL_U *doepctl_ptr = (USB_DOEPCTL_U *) USB_DOEPCTL (ep_num);

		doepctl_ptr->mBits.ep_type = ep_type;
		doepctl_ptr->mBits.mps =mps;
		doepctl_ptr->mBits.set_nak = 0x1;
	}
	else
	{
		volatile USB_DIEPCTL_U *diepctl_ptr = (USB_DIEPCTL_U *) USB_DIEPCTL (ep_num);

		diepctl_ptr->mBits.ep_type = ep_type;
		diepctl_ptr->mBits.mps = mps;
		diepctl_ptr->mBits.set_nak = 0x1;
	}
}

/*****************************************************************************/
//  Description:   start endpoint transfer.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_start_transfer (USB_EP_NUM_E ep_num, BOOLEAN dir, uint32 transfer_size, BOOLEAN is_dma, uint32 *buffer)
{
	uint16 packet_count = 0;

	if (dir)
	{
		volatile USB_DOEPTSIZ_U *doeptsiz_ptr = (USB_DOEPTSIZ_U *) USB_DOEPTSIZ (ep_num);

		if (is_dma)
		{
			* (volatile uint32 *) USB_DOEPDMA (ep_num) = (uint32) buffer;
		}

		doeptsiz_ptr->mBits.transfer_size = MAX_RECV_LENGTH;    // transfer size
		doeptsiz_ptr->mBits.packet_count = MAX_RECV_LENGTH/BULK_MPS;
		* (volatile uint32 *) USB_DOEPCTL (ep_num) |= (unsigned int) (BIT_26|BIT_31); // clear nak
	}
	else
	{
		volatile USB_DIEPTSIZ_U *dieptsiz_ptr = (USB_DIEPTSIZ_U *) USB_DIEPTSIZ (ep_num);
		volatile USB_DIEPCTL_U   *diepctl_ptr = (USB_DIEPCTL_U *) USB_DIEPCTL (ep_num);

		if (is_dma)
		{
			Dcache_CleanRegion((unsigned int)buffer,  transfer_size);//0511
			* (volatile uint32 *) USB_DIEPDMA (ep_num) = (uint32) buffer;
		}

		dieptsiz_ptr->mBits.transfer_size = transfer_size;                  // transfer size
		packet_count = (transfer_size+diepctl_ptr->mBits.mps-1) /diepctl_ptr->mBits.mps;/*lint !e564*/
		dieptsiz_ptr->mBits.packet_count = packet_count;                    // packet count
        diepctl_ptr->mBits.tx_fifo_number = ep_num;                         // tx fifo number

		* (volatile uint32 *) USB_DIEPCTL (ep_num) |= (unsigned int) BIT_26;            // clear nak
		* (volatile uint32 *) USB_DIEPCTL (ep_num) |= (unsigned int) BIT_31;            // endpoint enable
	}

}

/*****************************************************************************/
//  Description:   process desecriptor request.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_get_descriptor (USB_REQUEST_1_U *request1, USB_REQUEST_2_U *request2)
{
	uint32 length = 0;
	uint8 *send_data=NULL;
	uint8 pkt_cnt=0;
	uint32 config_des_size=0;

	length = (uint32) (request2->mBits.length_m<<8 |request2->mBits.length_l);

	switch (request1->mBits.value_m)
	{
		case USB_DEVICE_DESCRIPTOR_TYPE:
			send_data = (uint8 *) thor_get_device_desc(enum_speed);

			EPI0_config (0x12, 0x1, TRUE, (uint32 *) send_data);
			break;

		case USB_CONFIGURATION_DESCRIPTOR_TYPE:
			send_data = (uint8 *) thor_get_config_desc(enum_speed);
			config_des_size = send_data[3]<<8 |send_data[2];

			if (length > config_des_size)
			{
				pkt_cnt =  (config_des_size % 64) ? config_des_size/64 + 1 : config_des_size/64;
				EPI0_config (config_des_size, pkt_cnt, TRUE, (uint32 *) send_data);
			}
			else
			{
				pkt_cnt =  (length % 64) ? length/64 + 1 : length/64;
				EPI0_config (length, pkt_cnt, TRUE, (uint32 *) send_data);
			}
			break;

		case USB_STRING_DESCRIPTOR_TYPE:
		{
			uint8 str_index = request1->mBits.value_l;
			send_data = thor_get_string_desc(str_index);

			if(length > send_data[0])
			{
				EPI0_config (send_data[0], 0x1, TRUE, (uint32 *) send_data);
			}
			else
			{
				EPI0_config (length, 0x1, TRUE, (uint32 *) send_data);
			}
		}
			break;

		case USB_DEVICE_QUALIFIER_TYPE:
			send_data = (uint8 *) thor_get_qualifer_desc();
			if(length > send_data[0])
			{
				EPI0_config (send_data[0], 0x1, TRUE, (uint32 *) send_data);
			}
			else
			{
				EPI0_config (length, 0x1, TRUE, (uint32 *) send_data);
			}
			break;

		case USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE:

			send_data = (uint8 *) thor_get_other_speed_config_desc();
			config_des_size = send_data[3]<<8 |send_data[2];
			if (length > config_des_size)
			{
				pkt_cnt =  (config_des_size % 64) ? config_des_size/64 + 1 : config_des_size/64;
				EPI0_config (config_des_size, pkt_cnt, TRUE, (uint32 *) send_data);
			}
			else
			{
				pkt_cnt =  (length % 64) ? length/64 + 1 : length/64;
				EPI0_config (length, pkt_cnt, TRUE, (uint32 *) send_data);
			}
			break;

		default:
			break;
	}
}
/*****************************************************************************/
//  Description:   process setup transaction.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL int USB_EP0RecvEmptyPacket (void)
{
	uint32 packet_count;
	volatile USB_DOEPINT_U *doepint_ptr = (USB_DOEPINT_U *) USB_DOEPINT (0);

	if(doepint_ptr->mBits.transfer_com == 1)
		doepint_ptr->mBits.transfer_com=1;
	packet_count = 1;
	* (volatile uint32 *) USB_DOEP0TSIZ = (packet_count<<19) ; //setup packet count , 3 packet

	* (volatile uint32 *) USB_DOEP0CTL |= BIT_26;       // set clear NAK
	* (volatile uint32 *) USB_DOEP0CTL |= BIT_31;       // set endpoint enable
	while(doepint_ptr->mBits.transfer_com==0)
	{
		doepint_ptr = (USB_DOEPINT_U *) USB_DOEPINT (0);
	}
	return 0;
}
LOCAL int USB_EP0RecvData (uint32 *pBuf,int transfer_size)
{
	uint32 packet_count;

	if (transfer_size == 0)
	{
		return 0;
	}

	packet_count = (transfer_size+63) /64;
	* (volatile uint32 *) USB_DOEP0TSIZ = (packet_count<<19) | (transfer_size&0x7f); //setup packet count , 3 packet
	* (volatile uint32 *) USB_DOEPDMA (0) = (uint32) pBuf;
	* (volatile uint32 *) USB_DOEP0CTL |= BIT_26;       // set clear NAK
	* (volatile uint32 *) USB_DOEP0CTL |= BIT_31;       // set endpoint enable
	return 0;
}
LOCAL void usb_reset_pipe(uint8 epno)
{
	uint8 ep_num = epno & 0x7F;
	if(ep_num == 0)
	{
		return;
	}
	if((epno & 0x80) == 0)
	{
		*(volatile uint32 *)USB_DOEPCTL(ep_num) |= BIT_28;
	}
	else
	{
		*(volatile uint32 *)USB_DIEPCTL(ep_num) |= BIT_28;
	}
}
/*****************************************************************************/
//  Description:   process setup transaction.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
PUBLIC void usb_set_feature (void)
{
	volatile USB_DCFG_U *dcfg_ptr = (volatile USB_DCFG_U *) USB_DCFG;
	volatile USB_DCTL_U *dctl_ptr = (volatile USB_DCTL_U *) USB_DCTL;
	USB_REQUEST_1_U   *request1;
	USB_REQUEST_2_U   *request2;
	USB_REQUEST_1_U   request1_u;
	USB_REQUEST_2_U   request2_u;
	request1_u.dwValue = s_setup_packet[0];
	request2_u.dwValue = s_setup_packet[1];
	request1= &request1_u;
	request2= &request2_u;

	EPI0_config (0x0, 0x1, FALSE, NULL);

	while (* (volatile uint32 *) USB_DIEP0TSIZ) {}; // wait packet count is zero

	* (volatile uint32 *) USB_DOEP0CTL |= BIT_26; // clear ep out nak

	switch (request2->mBits.index_m)
	{
		case    1://Test_J
			dctl_ptr->mBits.tstctl = request2->mBits.index_m;
			break;
		case    2://Test_K
			dctl_ptr->mBits.tstctl = request2->mBits.index_m;
			break;
		case    3://Test_SE0_NAK
			dctl_ptr->mBits.tstctl = request2->mBits.index_m;
			break;
		case    4://Test_Packet
			dctl_ptr->mBits.tstctl = request2->mBits.index_m;
			break;
		case    5://Test_Force_Enable
			dctl_ptr->mBits.tstctl = request2->mBits.index_m;
			break;
		default:
			break;
	}

	dctl_ptr->mBits.tstctl = request2->mBits.index_m;
}

LOCAL void usb_setup_handle (void)
{
	usb_debug("%s : enter \n", __func__);
	uint32  vendor_ack = 0;
	USB_REQUEST_1_U   *request1;
	USB_REQUEST_2_U   *request2;
	USB_REQUEST_1_U   request1_u;
	USB_REQUEST_2_U   request2_u;
	request1_u.dwValue = s_setup_packet[0];
	request2_u.dwValue = s_setup_packet[1];
	request1= &request1_u;
	request2= &request2_u;

	switch (request1->mBits.type)
	{
		case USB_REQ_STANDARD://standard

			switch (request1->mBits.recipient)						//Recipient
			{
				case USB_REC_DEVICE:

					switch (request1->mBits.brequest)
					{
						case USB_REQUEST_SET_FEATURE:
							usb_set_feature ();
							break;
						case USB_REQUEST_SET_ADDRESS:
						{
							volatile USB_DCFG_U *dcfg_ptr = (USB_DCFG_U *) USB_DCFG;

							dcfg_ptr->mBits.devaddr = request1->mBits.value_l;
							EPI0_config (0, 1, FALSE, NULL);
						}
							break;
						case USB_REQUEST_GET_DESCRIPTOR:
							usb_get_descriptor (request1, request2);
							break;
						case USB_REQUEST_SET_CONFIGURATION: //0x00 0x09
							g_cdc_configuration = request1->mBits.value_l;
							EPI0_config (0, 1, FALSE, NULL);

							if(g_cdc_status ==0)
							{
								usb_EPActive (EP_IN, USB_EP_DIR_IN);
								usb_EPActive (EP_OUT, USB_EP_DIR_OUT);
								g_cdc_status = 1;
							}
							break;
						case USB_REQUEST_GET_CONFIGURATION:
							EPI0_config (request2->mBits.length_l, 1, TRUE, &g_cdc_configuration);
							break;
						case USB_REQUEST_SET_INTERFACE:
							EPI0_config (0, 1, FALSE, NULL);
							break;
						case USB_REQUEST_CLEAR_FEATURE:
							if(request1->mBits.recipient == 2)
							{
								usb_reset_pipe(request2->mBits.index_l);
							}
							EPI0_config (0, 1, FALSE, NULL);
							break;
						default:
							EPI0_config (0, 1, TRUE, &vendor_ack);
							break;
					}
					break;

				case USB_REC_INTERFACE:

					switch(request1->mBits.brequest)
					{
						case USB_REQUEST_SET_INTERFACE://0x01 0x0b
							g_cdc_interface = request1->mBits.value_l;
							EPI0_config (0, 1, FALSE, NULL);
							break;
						case USB_REQUEST_GET_INTERFACE://0x81 0x0a
							EPI0_config (request2->mBits.length_l, 1, TRUE, &g_cdc_interface);
							break;
					}
					break;

				case USB_REC_ENDPOINT:

					switch(request1->mBits.brequest)
					{
						case USB_REQUEST_GET_STATUS://0x82 0x00
							EPI0_config (request2->mBits.length_l, 1, TRUE, &g_cdc_state);
							break;
						case USB_REQUEST_SET_FEATURE://0x02 0x03
							g_cdc_state = 1;
							usb_set_feature ();
							break;
						case USB_REQUEST_CLEAR_FEATURE://0x02 0x01
							if(request1->mBits.recipient == 2)
							{
								usb_reset_pipe(request2->mBits.index_l);
							}
							EPI0_config (0, 1, FALSE, NULL);
							g_cdc_state = 0;
							break;
						default:
							break;
					}
					break;
			}
			break;

		case USB_REQ_CLASS://class
			switch (request1->mBits.recipient)
			{
				case USB_REC_INTERFACE:
					switch (request1->mBits.brequest)
					{
						case 0x22:
							if (request1->mBits.value_l)
							{
								if (g_cdc_status == 0)
								{
									usb_EPActive (EP_IN, USB_EP_DIR_IN);
									usb_EPActive (EP_OUT, USB_EP_DIR_OUT);
									g_cdc_status = 1;
									usb_debug("g_cdc_status = 1 \n");
								}
							}
							EPI0_config (0, 1, FALSE, NULL);
							break;
						case 0x20:
							if (request2->mBits.length_l)
							{
								USB_EP0RecvData((uint32 *)line_coding_config,request2->mBits.length_l);
							}
								EPI0_config (0, 1, FALSE, NULL);
							break;
						case 0x21:
							EPI0_config(7, 1, TRUE,(uint32 *)line_coding_config);
							USB_EP0RecvEmptyPacket();
							break;
						case USB_CLEAR_COMM_FEATURE :
							s_comm_feature = 0;
							EPI0_config (0, 1, FALSE, NULL);
							break;
						case USB_GET_COMM_FEATURE :
							if (request2->mBits.length_l)
							{
								EPI0_config (2, 1, TRUE, &s_comm_feature);
							}
							break;
						case USB_SET_COMM_FEATURE :
							if (request2->mBits.length_l)
							{
								USB_EP0RecvData((uint32 *)&s_comm_feature,2);
							}
							EPI0_config (0, 1, FALSE, NULL);
							break;
					}
					break;
				default:
					EPI0_config (0, 1, TRUE, NULL);
					break;
			}
			break;
		case USB_REQ_VENDOR:
			EPI0_config (4, 1, TRUE, &vendor_ack);
			break;
		default:
			EPI0_config (0, 1, TRUE, NULL);
			break;
	}
}
/*****************************************************************************/
//  Description:   usb reset interrupt handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/

LOCAL void usb_reset_handler (void)
{
	uint32  timeout = 0;
	volatile USB_DCFG_U *dcfg_ptr = (USB_DCFG_U *) USB_DCFG;

    dcfg_ptr->mBits.devaddr = 0;
    *(volatile uint32 *) USB_GAHBCFG |= BIT_5;
    *(volatile uint32 *) USB_GINTMSK &= (unsigned int) (~BIT_12);                          // disable reset interrupt

    *(volatile uint32 *) USB_DOEP0CTL |= (unsigned int) BIT_27;                            // set NAK for all OUT endpoint

    *(volatile uint32 *) USB_DOEPCTL (6) |= (unsigned int) BIT_27;

    *(volatile uint32 *) USB_DAINTMSK |= (unsigned int) (BIT_0|BIT_16);
    *(volatile uint32 *) USB_DOEPMSK |= (unsigned int) (BIT_0|BIT_3|BIT_2|BIT_1);
    *(volatile uint32 *) USB_DIEPMSK |= (unsigned int) (BIT_0|BIT_3|BIT_1|BIT_2|BIT_5);//lint !e718

	*(volatile uint32 *) USB_GRXFSIZ = (unsigned int) (BIT_2 | BIT_4 | BIT_8);
	*(volatile uint32 *) USB_GNPTXFSIZ = (unsigned int) ((BIT_2 | BIT_4 | BIT_8) | BIT_20);
	*(volatile uint32 *) USB_DIEPTXF (1) = (unsigned int) ((BIT_2 | BIT_5 | BIT_8) | BIT_24);
	*(volatile uint32 *) USB_DIEPTXF (2) = (unsigned int) ((BIT_2 | BIT_5 | BIT_9) | BIT_18);
	*(volatile uint32 *) USB_DIEPTXF (3) = (unsigned int) ((BIT_3 | BIT_5 | BIT_9) | BIT_24);
	*(volatile uint32 *) USB_DIEPTXF (4) = (unsigned int) ((BIT_3 | BIT_5 | BIT_8 | BIT_9) | BIT_18);
	*(volatile uint32 *) USB_DIEPTXF (5) = (unsigned int) ((BIT_2 | BIT_3 | BIT_5 | BIT_8 | BIT_9) | BIT_24);
	*(volatile uint32 *) USB_DIEPTXF (6) = (unsigned int) ((BIT_2 | BIT_3 | BIT_5 | BIT_10) | BIT_18);

    *(volatile uint32 *) USB_GRSTCTL |= (unsigned int) BIT_5;                          //reflush tx fifo

	while ( (* (volatile uint32 *) USB_GRSTCTL) & ( (unsigned int) BIT_5))
	{
		timeout++;

		if (timeout >= USB_TIMEOUT)
		{
			break;
		}
	}

	timeout = 0;

	* (volatile uint32 *) USB_GRSTCTL |= (unsigned int) BIT_4;                          //reflush rx fifo

	while ( (* (volatile uint32 *) USB_GRSTCTL) & ( (unsigned int) BIT_4))
	{
		timeout++;

		if (timeout >= USB_TIMEOUT)
		{
			break;
		}
	}

	Dcache_InvalRegion((unsigned int)s_setup_packet, sizeof(s_setup_packet));//////////for test
	EPO0_config (TRUE, s_setup_packet);

	*(volatile uint32 *) USB_GINTMSK |= (unsigned int) BIT_12;                             // enable reset interrupt
	*(volatile uint32 *) USB_GINTSTS |= (unsigned int) BIT_12;                             //clear reset interrupt
}

/*****************************************************************************/
//  Description:   usb enumeration done handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/

LOCAL void usb_enumeration_done (void)
{
	volatile USB_DSTS_U *dsts_ptr = (USB_DSTS_U *) USB_DSTS;

	enum_speed = dsts_ptr->mBits.enumspd;                                   //read enumration speed
	* (volatile uint32 *) USB_DIEP0CTL &= (unsigned int) (~ (BIT_0|BIT_1));

	if ( enum_speed == USB_HIGH )
	{
		EPn_config (EP_IN, USB_EP_TYPE_BULK, USB_EP_DIR_IN, BULK_MPS);
		EPn_config (EP_OUT, USB_EP_TYPE_BULK, USB_EP_DIR_OUT, BULK_MPS);
		EPn_config (EP_INTERRUPT, USB_EP_TYPE_INTERRUPT, USB_EP_DIR_IN, USB_PACKET_16);
	}
	else
	{
		EPn_config (EP_IN, USB_EP_TYPE_BULK, USB_EP_DIR_IN, USB_PACKET_64);
		EPn_config (EP_OUT, USB_EP_TYPE_BULK, USB_EP_DIR_OUT, USB_PACKET_64);
		EPn_config (EP_INTERRUPT, USB_EP_TYPE_INTERRUPT, USB_EP_DIR_IN, USB_PACKET_16);
	}
	usb_EPActive (EP_INTERRUPT, USB_EP_DIR_IN);
	Dcache_InvalRegion((unsigned int)s_setup_packet, sizeof(s_setup_packet));
	EPO0_config (TRUE, s_setup_packet);

	*(volatile uint32 *) USB_DCTL |= (unsigned int) BIT_8;
	*(volatile uint32 *) USB_GINTSTS |= (unsigned int) BIT_13;
}

/*****************************************************************************/
//  Description:   out endpoint0 handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/

LOCAL void usb_EP0_out_handle (void)
{
	volatile USB_DOEPINT_U *doepint_ptr = (USB_DOEPINT_U *) USB_DOEPINT (0);
	USB_DOEPINT_U doepint;

	doepint.dwValue = doepint_ptr->dwValue;
	doepint_ptr->dwValue = doepint.dwValue;

	if (doepint.mBits.timeout_condi)
	{
		usb_setup_handle ();
	}

	if (doepint.mBits.transfer_com)
	{
		uint8 size = 0;
		uint32 i;
		*(volatile uint32 *) USB_DOEP0CTL |= (unsigned int) BIT_27;

		for(i=0;i<50;i++);

		size = (*(volatile uint32 *)USB_DOEP0TSIZ) & 0x18;
		if(size != 0x18)
		{
			usb_setup_handle();
		}
	}

	Dcache_InvalRegion ((unsigned int) s_setup_packet, sizeof (s_setup_packet));	//0511
	EPO0_config (TRUE, s_setup_packet);	//renable ep0 nd set packet count
}


/*****************************************************************************/
//  Description:   out endpoint handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_EP_out_handle (void)
{
	volatile USB_DAINT_U *daint_ptr = (USB_DAINT_U *) USB_DAINT;
	USB_DAINT_U daint;

	daint.dwValue = daint_ptr->dwValue;         // disable EP out interrupt

	if (daint.mBits.outepint_0)
	{
		usb_EP0_out_handle();
	}

	//* (volatile uint32 *) USB_GINTMSK |= (unsigned int) BIT_19; // enable reset interrupt
}
/*****************************************************************************/
//  Description:   in endpoint handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_EP_in_handle (void)
{
	volatile USB_DAINT_U *daint_ptr = (USB_DAINT_U *) USB_DAINT;
	USB_DAINT_U daint;

	daint.dwValue = daint_ptr->dwValue;

	if (daint.mBits.inepint_0)
	{
		volatile USB_DIEPINT_U *diepint_ptr = (USB_DIEPINT_U *) USB_DIEPINT (0);
		USB_DIEPINT_U diepint;
		diepint.dwValue = diepint_ptr->dwValue;
		diepint_ptr->dwValue = diepint.dwValue;
	}
}

/*****************************************************************************/
//  Description:   usb interrupt handler.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
static  void usb_handler (void)
{
	volatile USB_INTSTS_U *usb_int_ptr = (USB_INTSTS_U *) USB_GINTSTS;
	volatile USB_INTSTS_U  usb_int;
	uint32 i=0;
         char string[64] ={0,};
	usb_int.dwValue = usb_int_ptr->dwValue;

	// in endpoint interrupt
	if (usb_int.mBits.iepint)
		usb_EP_in_handle();

	// out endpoint interrupt
	if (usb_int.mBits.oepint)
		usb_EP_out_handle();

	// enumeration done interrupt
	if (usb_int.mBits.enumdone)
		usb_enumeration_done();

	// reset interrupt
	if (usb_int.mBits.usbrst)
	{
		if (g_cdc_status)
		{
			check_usb_reconnected = 1;
			g_cdc_status = 0;
		}
		usb_reset_handler();
	}

	if (usb_int.mBits.usbsusp)
	{
		if(g_usb_status == USB_STARTED)
		{
			g_usb_status = USB_SUSPEND_AFTER_USB_MODE_START;
		}
	}
}

/*****************************************************************************/
//  Description:   configure in endpoint ep_id to send message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
//extern int microusb_charger_connected(void);
extern u32 is_usb_cable(void);
static int32 USB_ReadEx_Internal(uint8 *pBuf,uint32 len)
{
    volatile USB_DOEPINT_U *doepint_ptr = (USB_DOEPINT_U *) USB_DOEPINT (EP_OUT);
    volatile USB_DOEPTSIZ_U *doeptsiz_ptr = (USB_DOEPTSIZ_U *) USB_DOEPTSIZ (EP_OUT);
    int i;

    if(recv_length == 0)
    {
        if(doepint_ptr->mBits.transfer_com == 1)
        {
            doepint_ptr->mBits.transfer_com = 1;
        }

        Dcache_CleanRegion((unsigned int)(&pBuf[0]),  MAX_RECV_LENGTH); 
        usb_start_transfer (EP_OUT, USB_EP_DIR_OUT, 1, TRUE, (uint32 *) pBuf);
        while(doepint_ptr->mBits.transfer_com==0)
        {
		doepint_ptr = (USB_DOEPINT_U *) USB_DOEPINT (EP_OUT);
		usb_handler();
		if(check_usb_reconnected == 1 && g_cdc_status == 1)
		{
			usb_start_transfer (EP_OUT, USB_EP_DIR_OUT, 1, TRUE, (uint32 *) pBuf);
			check_usb_reconnected = 0;
		}
	}
	doepint_ptr->mBits.transfer_com = 1;
	Dcache_InvalRegion((unsigned int)(&pBuf[0]),  MAX_RECV_LENGTH);//0511
	recv_length = MAX_RECV_LENGTH - doeptsiz_ptr->mBits.transfer_size;
    }

    if(recv_length > len)
    {
	recv_length -= len;
    }
    else
    {
	len = recv_length;
	recv_length = 0;
    }

    return len;
}

int USB_ReadEx(unsigned char *pBuf, int len)
{
    int32 ret = len;
    int32 read_len = 0;
    if (len == 0)
		return 0;
    do {
        read_len = USB_ReadEx_Internal(pBuf,len);
        if( read_len == 6 )
        {
            /* RDX exception handling when PC(intenet explorer) control RDX tool */
            return 6;
        }

        if( g_usb_status == USB_DETACHED_AFTER_USB_MODE_START )
        {
            return 0;
        }
        pBuf += read_len;
        len -= read_len;
    } while (len > 0);

    return ret;
}
/*****************************************************************************/
//  Description:   configure in endpoint ep_id to send message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
PUBLIC int32 USB_WriteEx(uint8 *pBuf,uint32 len)
{
	volatile USB_DIEPINT_U *diepint_ptr = (USB_DIEPINT_U *) USB_DIEPINT (EP_IN);
	int i;

	int transfer_size = 0;
	int transfered_size = 0;


	do{
		if(len > MAX_RECV_LENGTH)
		{
			transfer_size = MAX_RECV_LENGTH;
		}
		else if((len % MAX_PACKET_SIZE)==0)
		{
			transfer_size = len - 32;
		}
		else
		{
			transfer_size = len;
		}

		len = len - transfer_size;

		for(i=0;i<transfer_size;i++)
		{
			usb_out_endpoint_buf[1][i] = pBuf[i+transfered_size];
		}

		if(diepint_ptr->mBits.transfer_com == 1)
		{
			diepint_ptr->mBits.transfer_com = 1;
		}
		usb_handler();
		usb_start_transfer ( (USB_EP_NUM_E) EP_IN, USB_EP_DIR_IN, transfer_size, TRUE, (uint32 *) usb_out_endpoint_buf[1]);

		do{
			diepint_ptr = (USB_DIEPINT_U *) USB_DIEPINT (EP_IN);
			usb_handler();
		}while(diepint_ptr->mBits.transfer_com==0);
		diepint_ptr->mBits.transfer_com = 1;
		transfered_size += transfer_size;
	}while(len >0);
	return transfered_size;
}

/*****************************************************************************/
//  Description:   initialize the usb core.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/
LOCAL void usb_core_init (void)
{
	usb_debug("%s : enter \n", __func__);
	uint32 time_out=0;

	//Core soft reset, include hclk and phy clock
	* (volatile uint32 *) USB_GRSTCTL |= BIT_0;
	usb_debug("Inside usb_core_init \n");
	do {
		uint32 reg_val = 0;
		time_out++;
		reg_val = * (volatile uint32 *) USB_GRSTCTL;

		if (reg_val & BIT_31)
		{
			break;
		}
	} while (time_out<10);

	* (volatile uint32 *) USB_DCTL &= ~ (BIT_1);  //soft disconnect
	* (volatile uint32 *) USB_GOTGCTL |= BIT_6|BIT_7;//Bvalid en

	// program global ahb configuration

	* (volatile uint32 *) USB_GAHBCFG |= BIT_1|BIT_2|BIT_3;     // burst length  INCR16
	* (volatile uint32 *) USB_GAHBCFG |= BIT_0;                 // global interrupt mask  , 0:mask 1:unmask

	// program global usb configuration
	* (volatile uint32 *) USB_GUSBCFG &= ~ (BIT_6);             // External FS PAY or Interal FS  Serial PHY Selection, UTMI+ or ULPI PHY
	* (volatile uint32 *) USB_GUSBCFG &= ~ (BIT_17);            // External FS PAY or Interal FS  Serial PHY Selection, UTMI+ or ULPI PHY
	* (volatile uint32 *) USB_GUSBCFG &= ~ (BIT_4);             // ULPI or UTMI+ selection bit,  UTMI+ interface
	* (volatile uint32 *) USB_GUSBCFG |= BIT_3;                 // PHY Interface bit, 16 bit
	* (volatile uint32 *) USB_GUSBCFG &= ~ (BIT_0|BIT_1|BIT_2); // HS/FS time out calibration,
	* (volatile uint32 *) USB_GUSBCFG |= BIT_10|BIT_12;         // USB turnaround time, 16bit UTMI+
	* (volatile uint32 *) USB_GUSBCFG |= BIT_30;         // force device mode

	* (volatile uint32 *) USB_GINTSTS = 0xffffffff;

	* (volatile uint32 *) USB_GINTMSK =  0;                     // mask all first

	// device init
	* (volatile uint32 *) USB_DCFG &= ~BIT_2;                   // out handshake
	* (volatile uint32 *) USB_DCFG &= ~ (BIT_11 | BIT_12);

	* (volatile uint32 *) USB_GINTMSK |= BIT_12;                // usb reset int mask, 0:mask 1:unmask
	* (volatile uint32 *) USB_GINTMSK |= BIT_13;                // enumeration done int mask, 0:mask 1:unmask
	* (volatile uint32 *) USB_GINTMSK |= BIT_18;
	* (volatile uint32 *) USB_GINTMSK |= BIT_19;

	* (volatile uint32 *) USB_GINTMSK &= ~BIT_4;                // DMA mode, must mask rx fifo level interrupt

	* (volatile uint32 *) USB_DCFG &= ~BIT_0;  //configure HS mode.
	usb_debug("After USBC_CORE_INIT \n");
}
/*****************************************************************************/
//  Description:   configure in endpoint ep_id to send message.
//  Global resource dependence:
//  Author:        jiayong.yang
//  Note:
/*****************************************************************************/

/* Enter point */
void thor_USB_Init(void)
{
	usb_debug("%s : enter\n", __func__);
	unsigned int len = 0;

	usb_power_on();
	usb_core_init();
	usb_debug("usb_Core_init() is called\n");

USB_RESTART:
	while (!g_cdc_status)
		usb_handler();

	sprd_usb_thor_start();

	if (g_usb_status == USB_DETACHED_AFTER_USB_MODE_START) {
		g_usb_status = USB_BEFORE_USB_MODE;
		g_cdc_status = 0;
		goto USB_RESTART;
	}
}

void USB_DeInit (void)
{
	usb_power_off();
}

__align(64) unsigned char sprd_thor_setup_buf[512] = {0,};

static int s_usb_connected = 0;

/************************************************
 *
 * Register call back function for v3_protocol.
 *
 ************************************************/
/* FIXME */
//extern int usb_cb_register(u32 (*up)(void *, u32), u32 (*down)(void *, u32));
#if 0
void sprd_usb_cb_register(void)
{
	usb_debug("%s : start \n", __func__);
	usb_cb_register(USB_WriteEx, USB_ReadEx);
}
#endif

extern int thor_handle(void);

void sprd_usb_thor_start()
{
    g_usb_status = USB_STARTED;

	do {
		if (s_usb_connected == 0) {
			USB_ReadEx_Internal(sprd_thor_setup_buf, sizeof(sprd_thor_setup_buf));

			if (g_usb_status == USB_DETACHED_AFTER_USB_MODE_START)
				return -1;

			if (!strncmp(sprd_thor_setup_buf, "THOR", strlen("THOR")))
			{
				usb_debug("- thor is connected!\n");
				USB_WriteEx("ROHT", strlen("ROHT"));
				usb_debug("thor Setup Complete\n");
				s_usb_connected = 1;
				break;
			} else{
				usb_debug("thor_seup error - Not receiving THOR\n");
				s_usb_connected = 0;
			}
		}
	} while(1);

	thor_handle();

	usb_debug("%s : exit \n", __func__);
	return 0;
}

void disconnect_usb(void)
{
	s_usb_connected = 0;
}

int thor_usb_is_connected(void)
{
	return s_usb_connected;
}
