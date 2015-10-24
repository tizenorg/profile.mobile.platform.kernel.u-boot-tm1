#
#


include $(TOPDIR)/config.mk

LIB	:= $(obj)libsprdfb_dsi.o

ifndef CONFIG_DSIH_VERSION_1P21A
#COBJS-$(CONFIG_DSIH_VERSION_1P10A) += mipi_dsih_api.o mipi_dsih_dphy.o mipi_dsih_hal.o
COBJS-y += mipi_dsih_api.o mipi_dsih_dphy.o mipi_dsih_hal.o
endif


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
