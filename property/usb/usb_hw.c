#include <config.h>
#include <common.h>
#include <asm/arch/sci_types.h>
#include <asm/arch/ldo.h>
//#include <asm/arch/chip_x35/sprd_reg_ap_apb.h>

/******************************************************************************
usb_hw.c 
******************************************************************************/
#define ADI_CLK_DIV                     (0X40030000 + 0x0000)	//(0x82000000 + 0x0000)
#define ADI_CTL_REG                     (0x40030000 + 0x0004)	//(0x82000000 + 0x0004) 
#define ADI_CHANNEL_PRI                 (0x40030000 + 0x0008)	//(0x82000000 + 0x0008) 
#define ADI_INT_EN                      (0x40030000 + 0x000C)
#define ADI_INT_RAW_STS                 (0x40030000 + 0x0010)
#define ADI_INT_MASK_STS                (0x40030000 + 0x0014)
#define ADI_INT_CLR                     (0x40030000 + 0x0018)
#define ADI_ARM_RD_CMD                  (0x40030000 + 0x0024)
#define ADI_RD_DATA                     (0x40030000 + 0x0028)
#define ADI_FIFO_STS                    (0x40030000 + 0x002C)
#define ADI_STS                         (0x40030000 + 0x0030)
#define ADI_REQ_STS                     (0x40030000 + 0x0034)

#define GR_GEN0                         (0x4B000000+ 0x0008)	//(0x8B000000 + 0x0008)	
#define GR_SOFT_RST                     (0x4B000000+ 0x004C)	//(0x8B000000 + 0x004C) 
#define GR_CLK_GEN5                     (0x4B000000 + 0x007C)	//(0x8B000000 + 0x007C) 

/* TODO : temp work , need to rearrange */
#if defined(CONFIG_SC9630)
#define AHB_CTRL0		        (0x20E00000)
#define AHB_SOFT_RST			(0x20E00004)
#define USB_PHY_CTRL            (0x20E0+0x3020)
#else
#define AHB_CTRL0		        (0x20D00000)
#define AHB_SOFT_RST			(0x20D00004)
#define USB_PHY_CTRL            (0x71300000+0x3000)
//#define USB_PHY_CTRL            (AHB_REG_BASE + 0xA0)
#endif
#define AHB_CONTROL_REG3	    (0x2090020c)
#define LDO_PD_CTRL			(0x82000000 + 0x0610)
#define APB_POWER_CONTROL       (0x402E0000 +0x60)
#define ADI_EB                  (0x00000040)
#define CLK_USB_REF_EN			(0x00000040)
#define CLK_USB_REF_SEL			(0x00000002)
#define USB_M_HBIGENDIAN		(0x00000004)
#define USBPHY_SOFT_RST			(0x00000080)
#define USBD_EN			        (0x00000020)
#define LDO_BPUSBH			    (0x00000001)
#define LDO_BPUSBH_RST			(0x00000002)
#define ADI_SOFT_RST            (0x00400000)
#define ARM_SERCLK_EN           (0x00000002)

#define mdelay(_ms)				udelay(_ms*1000)

static void ADIConfig(void)
{
    *(volatile uint32 *)GR_GEN0 |= ADI_EB;
    *(volatile uint32 *)GR_SOFT_RST |= ADI_SOFT_RST;
    {
    	uint32 wait;
    	for(wait=0;wait<50;wait++);
    }
    *(volatile uint32 *)GR_SOFT_RST &= ~ADI_SOFT_RST;
    *(volatile uint32 *)ADI_CTL_REG &= ~ARM_SERCLK_EN;
    *(volatile uint32 *)ADI_CHANNEL_PRI &=(0x00005555);
 	   	
}
static unsigned int ADIAnalogdieRegRead(unsigned long addr)
{
    uint32 adi_rd_data ;
    uint32 time_out=0;

    
    *(uint32 *) ADI_ARM_RD_CMD = addr;

    do{
    	adi_rd_data = *(uint32 *) ADI_RD_DATA;
        time_out++;
    	mdelay(1);
    	if(time_out > 10)
    	    return 0xFFFFFFFF;
    }while(adi_rd_data & 0x80000000);
    return((uint16) (adi_rd_data & 0xFFFF));
}
static unsigned int ADIAnalogdieRegWrite(unsigned long addr,uint16 data)
{
    uint32 status;
    uint32 time_out=0;
    

    do{
    	status = *(uint32 *) ADI_FIFO_STS;
    	if(status & 0x400)
    	    break;
        time_out++;
    	mdelay(1);
    	if(time_out > 10)
    	    return 0xFFFFFFFF;

    }while(1);
    
    *(uint32 *) addr = data;
    return 0;
}

static void USBLdoEnable(unsigned char is_usb_lod_enabled)
{
	int ret = -1;

	if(is_usb_lod_enabled) {
		ret = LDO_TurnOnLDO(LDO_LDO_USB);
		printf("%s: sabin Enable, LDO_LDO_USB no is %d, ret = %d \n", __func__, LDO_LDO_USB, ret);
	} else {
		ret = LDO_TurnOffLDO(LDO_LDO_USB);
		printf("%s: sabin Disable, LDO_LDO_USB, no is %d ret = %d \n", __func__, LDO_LDO_USB, ret);
	}
}

static void usb_enable_module(int en)
{
    if (en){
        *(uint32 *) APB_POWER_CONTROL &= 0xFFFFFFFE; 
        *(uint32 *) AHB_CTRL0 |= BIT_4;	          
    }else {
        *(uint32 *) APB_POWER_CONTROL |= 0x01;	
        *(uint32 *) AHB_CTRL0 &= ~BIT_4;	
    }
}

void usb_power_on(void)
{
	printf("%s : enter\n", __func__);
	int ret =0;

	*(volatile uint32 *) USB_PHY_CTRL = 0x4407ae33;

    usb_enable_module(1);    //SEL_INDIA_pankaj.s4
    mdelay(10);              //SEL_INDIA_pankaj.s4

    USBLdoEnable(0); //USB LDO turn on //SEL_INDIA_pankaj.s4
    mdelay(10);
    USBLdoEnable(1); //USB LDO turn on //SEL_INDIA_pankaj.s4

    *(volatile uint32 *) AHB_CTRL0 |= BIT_4;  //SEL_INDIA_pankaj.s4	
    *(uint32 *) AHB_SOFT_RST |= (BIT_5|BIT_6|BIT_7);	//SEL_INDIA_pankaj.s4
    mdelay(10);    //SEL_INDIA_pankaj.s4
    *(uint32 *) AHB_SOFT_RST &= ~(BIT_5|BIT_6|BIT_7); //SEL_INDIA_pankaj.s4
    *(uint32 *) AHB_CTRL0 |= BIT_4;	//SEL_INDIA_pankaj.s4

	mdelay(20);
	printf("End of usb_power_on \n");
}


void usb_power_off(void)
{
	usb_enable_module(0);
	USBLdoEnable(0);
}
