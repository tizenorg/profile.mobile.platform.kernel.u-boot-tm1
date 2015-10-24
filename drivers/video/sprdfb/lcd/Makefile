#
#
#


include $(TOPDIR)/config.mk

LIB	:= $(obj)liblcd.o

#ifdef CONFIG_LCD
COBJS-$(CONFIG_FB_LCD_S6E8AA5X01_MIPI) += lcd_s6e8aa5x01_mipi.o
COBJS-$(CONFIG_FB_LCD_S6E88OA_MIPI) += lcd_s6e88oa_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8363_RGB_SPI) += lcd_hx8363_rgb_spi.o
COBJS-$(CONFIG_FB_LCD_HX8363_MCU) += lcd_hx8363_mcu.o
COBJS-$(CONFIG_FB_LCD_S6D0139) += lcd_s6d0139.o
COBJS-$(CONFIG_FB_LCD_NT35516_MCU) += lcd_nt35516_mcu.o
COBJS-$(CONFIG_FB_LCD_NT35516_MIPI) += lcd_nt35516_mipi.o
COBJS-$(CONFIG_FB_LCD_NT35516_RGB_I2C) += lcd_nt35516_rgb_i2c.o
COBJS-$(CONFIG_FB_LCD_NT35516_RGB_SPI) += lcd_nt35516_rgb_spi.o
COBJS-$(CONFIG_FB_LCD_NT35510_MIPI) += lcd_nt35510_mipi.o
COBJS-$(CONFIG_FB_LCD_NT35512_MIPI) += lcd_nt35512_mipi.o
COBJS-$(CONFIG_FB_LCD_OTM8018B_MIPI) += lcd_otm_8018b_mipi.o
COBJS-$(CONFIG_FB_LCD_SSD2075_MIPI) += lcd_ssd2075_mipi.o
COBJS-$(CONFIG_FB_LCD_OTM1283A_MIPI) += lcd_otm1283a_mipi.o
COBJS-$(CONFIG_FB_LCD_T8861_MIPI) += lcd_t8861_mipi.o
COBJS-$(CONFIG_FB_LCD_ILI9486)  += lcd_ili9486.o
COBJS-$(CONFIG_FB_LCD_ILI9486_RGB_SPI) += lcd_ili9486_rgb_spi.o
COBJS-$(CONFIG_FB_LCD_NT51017_LVDS) += lcd_nt51017_mipi_lvds.o
COBJS-$(CONFIG_FB_LCD_HX8369B_MIPI) += lcd_hx8369b_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8369B_MIPI_KIRAN3G) += lcd_hx8369b_mipi_kiran3g.o
COBJS-$(CONFIG_FB_LCD_HX8369B_MIPI_COREPRIMELITE) += lcd_hx8369b_mipi_coreprimelite.o
COBJS-$(CONFIG_FB_LCD_SD7798D_MIPI_COREPRIMELITE) += lcd_sd7798d_mipi.o
COBJS-$(CONFIG_FB_LCD_ILI9341)  += lcd_ili9341.o
COBJS-$(CONFIG_FB_LCD_ST7789V_MCU)  += lcd_st7789v_mcu.o
COBJS-$(CONFIG_FB_LCD_NT35502_MIPI)  += lcd_nt35502_mipi.o
COBJS-$(CONFIG_FB_LCD_VIVA_RGB_SPI)  += lcd_hx8363_rgb_spi_viva.o
COBJS-$(CONFIG_FB_LCD_SC7798_RGB_SPI)  += lcd_sc7798_rgb_spi.o
COBJS-$(CONFIG_FB_LCD_ILI9806E_MIPI)  += lcd_ili9806e_mipi.o
COBJS-$(CONFIG_FB_LCD_ILI9806E_2_MIPI)  += lcd_ili9806e_2_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8379A_MIPI)  += lcd_hx8379a_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8389C_MIPI) += lcd_hx8389c_mipi.o
COBJS-$(CONFIG_FB_LCD_ILI6150_LVDS) += lcd_ili6150_lvds.o
COBJS-$(CONFIG_FB_LCD_RM68180_MIPI) += lcd_rm68180_mipi.o
COBJS-$(CONFIG_FB_LCD_ILI9486S1_MIPI) += lcd_ili9486s1_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8369B_MIPI_VIVALTO_VE) += lcd_hx8369b_mipi_vivaltoVE.o
COBJS-$(CONFIG_FB_LCD_VX5B3D_MIPI) += lcd_vx5b3d_mipi.o
COBJS-$(CONFIG_FB_LCD_GRANDNEO_MIPI) += lcd_hx8369b_grandneo_mipi.o
COBJS-$(CONFIG_FB_LCD_TSHARK2_J3_MIPI) += lcd_hx8369b_tshark2_j3_mipi.o
COBJS-$(CONFIG_FB_LCD_SD7798D_MIPI) += lcd_sd7798d_mipi.o
COBJS-$(CONFIG_FB_LCD_S6D77A1_MIPI_PIKEA_J1) += lcd_s6d77a1_pikeA_mipi.o
COBJS-$(CONFIG_FB_LCD_OTM8019A_MIPI) += lcd_otm8019a_mipi.o
COBJS-$(CONFIG_FB_LCD_NT51017_MIPI) += lcd_nt51017_goyal_mipi.o
COBJS-$(CONFIG_FB_LCD_FL10802_MIPI) += lcd_fl10802_mipi.o
COBJS-$(CONFIG_FB_LCD_JD9161_MIPI) += lcd_jd9161_mipi.o
COBJS-$(CONFIG_FB_LCD_HX8394D_MIPI) += lcd_hx8394d_mipi.o

#endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
