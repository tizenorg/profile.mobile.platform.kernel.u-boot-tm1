#include <asm/arch/sci_types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/cdc.h>
#include <linux/byteorder/little_endian.h>

#define THOR_VENDOR_NUM			0x04E8
#define THOR_PRODUCT_NUM		0x685D

static struct usb_device_descriptor thor_device_desc_high __align (32)= {
	.bLength			= sizeof(thor_device_desc_high),
	.bDescriptorType	= USB_DT_DEVICE,

	.bcdUSB				= __constant_cpu_to_le16(0x0200),
	.bDeviceClass		= USB_CLASS_COMM,

	.bDeviceSubClass	= 0x02,
	.bDeviceProtocol	= 0x00,

	.bMaxPacketSize0	= 64,

	.idVendor			= __constant_cpu_to_le16(THOR_VENDOR_NUM),
	.idProduct			= __constant_cpu_to_le16(THOR_PRODUCT_NUM),
	.bcdDevice			= cpu_to_le16(0x021B),

	.iManufacturer		= 0x01,
	.iProduct			= 0x02,

	.iSerialNumber		= 0x00,
	.bNumConfigurations	= 0x01,
};

static struct usb_device_descriptor thor_device_desc_full __align (32) = {
	.bLength			= sizeof(thor_device_desc_full),
	.bDescriptorType	= USB_DT_DEVICE,

	.bcdUSB				= __constant_cpu_to_le16(0x0101),
	.bDeviceClass		= USB_CLASS_COMM,

	.bDeviceSubClass	= 0x02,
	.bDeviceProtocol	= 0x00,

	.bMaxPacketSize0	= 64,

	.idVendor			= __constant_cpu_to_le16(THOR_VENDOR_NUM),
	.idProduct			= __constant_cpu_to_le16(THOR_PRODUCT_NUM),
	.bcdDevice			= cpu_to_le16(0x021B),

	.iManufacturer		= 0x01,
	.iProduct			= 0x02,

	.iSerialNumber		= 0x00,
	.bNumConfigurations	= 0x01,
};

/* function descriptor */
static const struct usb_config_descriptor thor_config_desc __align (32) = {
	.bLength				= sizeof(thor_config_desc),
	.bDescriptorType		= USB_DT_CONFIG,

	.wTotalLength			= __constant_cpu_to_le16(67),

	.bNumInterfaces			= 0x02,
	.bConfigurationValue	= 0x01,
	.iConfiguration			= 0x00,

	.bmAttributes			= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower				= 0x19,
};

static struct usb_interface_descriptor thor_downloader_intf_init __align (32) = {
	.bLength				= sizeof(thor_downloader_intf_init),
	.bDescriptorType		= USB_DT_INTERFACE,

	.bInterfaceNumber		= 0,
	.bAlternateSetting		= 0,
	.bNumEndpoints			= 1,

	.bInterfaceClass		= USB_CLASS_COMM,

	.bInterfaceSubClass		= USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol		= USB_CDC_ACM_PROTO_AT_V25TER,
	.iInterface				= 3,
};

static const struct usb_cdc_header_desc thor_downloader_func_desc __align (32) = {
	.bLength				= sizeof(thor_downloader_func_desc),
	.bDescriptorType		= USB_DT_CS_INTERFACE,
	.bDescriptorSubType		= USB_CDC_HEADER_TYPE,
	.bcdCDC					= __constant_cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor thor_downloader_func_desc_call __align (32) = {
	.bLength				= sizeof(thor_downloader_func_desc_call),
	.bDescriptorType		= USB_DT_CS_INTERFACE,
	.bDescriptorSubType		= USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities			= 0x00,
	.bDataInterface			= 0x01,
};

static struct usb_cdc_acm_descriptor thor_downloader_func_desc_abstract __align (32) = {
	.bLength				= 0x04,
	.bDescriptorType		= USB_DT_CS_INTERFACE,
	.bDescriptorSubType		= USB_CDC_ACM_TYPE,
	.bmCapabilities			= 0x0F,
};

static struct usb_cdc_union_desc thor_downloader_cdc_union __align (32) = {
	.bLength				= 0x05,
	.bDescriptorType		= USB_DT_CS_INTERFACE,
	.bDescriptorSubType		= USB_CDC_UNION_TYPE,
	.bMasterInterface0		= 0x00,
	.bSlaveInterface0		= 0x01,
};

static struct usb_endpoint_descriptor thor_downloader_ep3_in __align (32) = {
	.bLength				= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType		= USB_DT_ENDPOINT,
	.bEndpointAddress		= 0x83,
	.bmAttributes			= USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize			= __constant_cpu_to_le16(16),
	.bInterval = 0x9,
};

static const struct usb_interface_descriptor thor_downloader_intf_data __align (32) = {
	.bLength				= USB_DT_INTERFACE_SIZE,
	.bDescriptorType		= USB_DT_INTERFACE,

	.bInterfaceNumber		= 1,
	.bAlternateSetting		= 0,

	.bNumEndpoints			= 2,
	.bInterfaceClass		= USB_CLASS_CDC_DATA,

	.bInterfaceSubClass		= 0,
	.bInterfaceProtocol		= 0,
	.iInterface				= 4,
};

static struct usb_endpoint_descriptor thor_downloader_ep1_in __align (32) = {
	.bLength			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= 0x81,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(512),
	.bInterval			= 0,
};

static struct usb_endpoint_descriptor thor_downloader_ep2_out __align (32) = {
	.bLength			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= 0x02,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(512),
	.bInterval			= 0,
};

static const struct usb_descriptor_header *thor_function_high[] __align (32) = {
	(struct usb_descriptor_header *) &thor_downloader_intf_init,
	(struct usb_descriptor_header *) &thor_downloader_func_desc,
	(struct usb_descriptor_header *) &thor_downloader_func_desc_call,
	(struct usb_descriptor_header *) &thor_downloader_func_desc_abstract,
	(struct usb_descriptor_header *) &thor_downloader_cdc_union,
	(struct usb_descriptor_header *) &thor_downloader_ep3_in,
	(struct usb_descriptor_header *) &thor_downloader_intf_data,
	(struct usb_descriptor_header *) &thor_downloader_ep1_in,
	(struct usb_descriptor_header *) &thor_downloader_ep2_out,
};

static struct usb_cdc_acm_descriptor thor_downloader_func_desc_abstract_full __align (32) = {
	.bLength				= 0x04,
	.bDescriptorType		= USB_DT_CS_INTERFACE,
	.bDescriptorSubType		= USB_CDC_ACM_TYPE,
	.bmCapabilities			= 0x00,
};

static struct usb_endpoint_descriptor thor_downloader_ep1_in_full __align (32) = {
	.bLength			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= 0x81,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(64),
	.bInterval			= 0,
};

static struct usb_endpoint_descriptor thor_downloader_ep2_out_full __align (32) = {
	.bLength			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= 0x02,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(64),
	.bInterval			= 0,
};

static const struct usb_descriptor_header *thor_function_full[] __align (32) = {
	(struct usb_descriptor_header *) &thor_downloader_intf_init,
	(struct usb_descriptor_header *) &thor_downloader_func_desc,
	(struct usb_descriptor_header *) &thor_downloader_func_desc_call,
	(struct usb_descriptor_header *) &thor_downloader_func_desc_abstract_full,
	(struct usb_descriptor_header *) &thor_downloader_cdc_union,
	(struct usb_descriptor_header *) &thor_downloader_ep3_in,
	(struct usb_descriptor_header *) &thor_downloader_intf_data,
	(struct usb_descriptor_header *) &thor_downloader_ep1_in_full,
	(struct usb_descriptor_header *) &thor_downloader_ep2_out_full,
};

static char function_desc_buf[67] __align(32);

#define CONFIG_THOR_STRING_PRODUCT			"SAMSUNG USB DRIVER"
static char string_product[2 + 2 * (sizeof(CONFIG_THOR_STRING_PRODUCT) - 1)] __align(32);

#define CONFIG_THOR_STRING_MANUFACTURER		"SAMSUNG"
static char string_manufacturer[2 + 2 * (sizeof(CONFIG_THOR_STRING_MANUFACTURER) - 1)] __align(32);

static char string_lang_ids[4] __align (32) = {4, USB_DT_STRING, 0x9, 0x4};

#define CONFIG_THOR_STRING_INTERFACE		"CDC Abstract Control Model"
static char string_interface[2 + 2 * (sizeof(CONFIG_THOR_STRING_INTERFACE) - 1)] __align(32);

#define CONFIG_THOR_STRING_CONTROL			"SAMSUNG SERIAL CONTROL"
static char string_control[2 + 2 * (sizeof(CONFIG_THOR_STRING_CONTROL) - 1)] __align(32);

static struct usb_qualifier_descriptor thor_dev_qualifier_desc __align(32) = {
	.bLength			= sizeof(thor_dev_qualifier_desc),
	.bDescriptorType	= USB_DT_DEVICE_QUALIFIER,
	.bcdUSB				= __constant_cpu_to_le16(0x0200),
	.bDeviceClass		= USB_CLASS_VENDOR_SPEC,

	.bDeviceSubClass	= 0x00,
	.bDeviceProtocol	= 0x00,
	.bMaxPacketSize0	= 64,

	.bNumConfigurations	= 0x01,
	.bRESERVED			= 0x00,
};

static char thor_device_qualifer_desc_buf[sizeof(struct usb_qualifier_descriptor)] __align(32);

unsigned char g_USB_Other_Speed_ConfigDescr[] __align(32) =
{
	0x09,0x07,0x43,0x00,0x02,0x01,0x00,0xC0,
	0x19,
	0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,
	0x03,
	0x05,0x24,0x00,0x10,0x01,
	0x05,0x24,0x01,0x00,0x01,
	0x04,0x24,0x02,0x0f,
	0x05,0x24,0x06,0x00,0x01,
	0x07,0x05,0x83,0x03,0x10,0x00,0x9,	//Ep 3 In	64
	0x09,0x04,0x01,0x00,0x02,0x0A,0x00,0x00,
	0x04,
	0x07,0x05,0x81,0x02,0x40,0x00,0x00,  //Ep 1 In  64    
	0x07,0x05,0x02,0x02,0x40,0x00,0x00,  //Ep 2 OUT  64			
};

unsigned char *thor_get_device_desc(unsigned int speed)
{
	if (speed == 0x00)		/* USB HIGH */
		return (unsigned char *) &thor_device_desc_high;
	else
		return (unsigned char *) &thor_device_desc_full;
}

unsigned char *thor_get_config_desc(unsigned int speed)
{
	usb_gadget_config_buf(&thor_config_desc, &function_desc_buf, 256, thor_function_high);

	if (speed == 0x00)		/* USB_HIGH */
		usb_gadget_config_buf(&thor_config_desc, &function_desc_buf, 256, thor_function_high);
	else
		usb_gadget_config_buf(&thor_config_desc, &function_desc_buf, 256, thor_function_full);

	return (unsigned char *) &function_desc_buf;
}

static void str2wide (char *str, u16 * wide)
{
	int i;
	for (i = 0; i < strlen (str) && str[i]; i++){
		#if defined(__LITTLE_ENDIAN)
			wide[i] = (u16) str[i];
		#elif defined(__BIG_ENDIAN)
			wide[i] = ((u16)(str[i])<<8);
		#else
			#error "__LITTLE_ENDIAN or __BIG_ENDIAN undefined"
		#endif
	}
}

unsigned char *thor_get_string_desc(unsigned char index)
{
	struct usb_string_descriptor *string;

	switch (index) {
		case 0:
			string = (struct usb_string_descriptor *) string_lang_ids;

			return (unsigned char *) string_lang_ids;
		case 1:
			string = (struct usb_string_descriptor *) string_manufacturer;
			string->bLength = sizeof(string_manufacturer);
			string->bDescriptorType = USB_DT_STRING;
			str2wide(CONFIG_THOR_STRING_MANUFACTURER, string->wData);

			return (unsigned char *) string;
		case 3:
			string = (struct usb_string_descriptor *) string_interface;
			string->bLength = sizeof(string_interface);
			string->bDescriptorType = USB_DT_STRING;
			str2wide(CONFIG_THOR_STRING_INTERFACE, string->wData);

			return (unsigned char *) string;
		case 6:
			string = (struct usb_string_descriptor *) string_control;
			string->bLength = sizeof(string_control);
			string->bDescriptorType = USB_DT_STRING;
			str2wide(CONFIG_THOR_STRING_CONTROL, string->wData);

			return (unsigned char *) string;
		case 2:
		default:
			string = (struct usb_string_descriptor *) string_product;
			string->bLength = sizeof(string_product);
			string->bDescriptorType = USB_DT_STRING;
			str2wide(CONFIG_THOR_STRING_PRODUCT, string->wData);

			return (unsigned char *) string;
	}

    return NULL;
}

unsigned char *thor_get_qualifer_desc(void)
{
    return (unsigned char *) &thor_dev_qualifier_desc;
}

unsigned char *thor_get_other_speed_config_desc(void)
{
    return (unsigned char *) g_USB_Other_Speed_ConfigDescr;
}
