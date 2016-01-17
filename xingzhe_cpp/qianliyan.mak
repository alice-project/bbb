include .config

CROSS_COMPILE =
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CROSS_COMPILE = arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2 
LDLIBS = -O -lrt -lpthread -ljpeg -liw -ldl -lv4l2
CFLAGS += -ffast-math -DLINUX -D_GNU_SOURCE -DUSE_LIBV4L2 -shared -fPIC
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CFLAGS += -mtune=cortex-a8 -march=armv7-a
LDLIBS += -L./ -L/usr/arm-linux-gnueabi/lib
endif
LDFLAGS =
INC = -I ../pub/ -I ../cross
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = v4l2uvc.o jpeg_utils.o dynctrl.o qianliyan.o

.PHONY: all
all: qianliyan.so

.PHONY: All
All: clean qianliyan.so

qianliyan.so: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o qianliyan.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	$(RM) *.o qianliyan.so

