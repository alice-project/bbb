include .config

CROSS_COMPILE =
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CROSS_COMPILE = arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2 
LDLIBS = -O -lrt -lpthread -ljpeg -liw -ldl
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CFLAGS += -mtune=cortex-a8 -march=armv7-a -ffast-math -DLINUX -D_GNU_SOURCE -DUSE_LIBV4L2 -fPIC -shared
LDLIBS += -L./ -L/usr/arm-linux-gnueabi/lib -L../cross
endif
LDFLAGS =
INC = -I ../pub/ -I ../cross
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = v4l2uvc.o jpeg_utils.o dynctrl.o input_uvc.o

.PHONY: all
all: input_uvc.so

.PHONY: All
All: clean input_uvc.so

input_uvc.so: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o input_uvc.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	$(RM) *.o input_uvc.so

