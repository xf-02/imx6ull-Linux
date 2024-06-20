
CROSS_COMPILE ?= 
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)/include/
CFLAGS += -I $(shell pwd)/ui/
# CFLAGS += -I $(shell pwd)/ui/

LDFLAGS := -lts -lpthread -lfreetype -lm -ljpeg

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := main

#obj-y += display/
#obj-y += test/
#obj-y += input/
#obj-y += font/
obj-y += ui/
obj-y += page/
#obj-y += config/
obj-y += sys/

all : start_recursive_build $(TARGET)
	@echo $(TARGET) has been built!

start_recursive_build:
#	make -j12  -C $(shell pwd)/ui/
#	make -j2 -C $(shell pwd)/driver/
#	make clean -C ./ -f $(TOPDIR)/Makefile.build
	make -C ./ -f $(TOPDIR)/Makefile.build

$(TARGET) : built-in.o
	$(CC) -o $(TARGET) built-in.o $(LDFLAGS)


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	