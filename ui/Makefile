#
# Makefile
#
EXTRA_CFLAGS  := 
CFLAGS_file.o := 

LVGL_DIR ?= ${shell pwd}
LVGL_DIR_NAME ?= lvgl

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

OBJEXT ?= .o

COBJS = $(CSRCS:.c=$(OBJEXT))

obj-y += $(COBJS)
obj-y += ui.o


