include .config

CROSS_COMPILE =
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CROSS_COMPILE = arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2 -Wno-pointer-arith -Wno-write-strings
LDLIBS = -O -lrt -lpthread -ljpeg -liw -ldl
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CFLAGS += -mtune=cortex-a8 -march=armv7-a -ffast-math 
LDLIBS += -L./ -L/usr/arm-linux-gnueabi/lib -L../cross
endif
LDFLAGS =
INC = -I ../pub/ -I ../cross
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = main.o xingzhe.o eye.o foot.o ear.o gpio_mmap.o i2c_func.o

.PHONY: all
all: xingzhe

.PHONY: All
All: clean xingzhe

xingzhe: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o xingzhe $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

qianliyan:
    make -f qianliyan.mak USE_LIBV4L2=TRUE

.PHONY: config
config:
	./gen-config-h.sh ./.config > xz_config.h

.PHONY: clean
clean:
	-$(RM) *.o xingzhe input_uvc.so output_udp.so output_http.so

