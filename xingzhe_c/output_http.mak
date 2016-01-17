include .config

CROSS_COMPILE =
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CROSS_COMPILE = arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2 
LDLIBS = -O -lrt -lpthread -ljpeg -liw -ldl
CFLAGS += -ffast-math -DLINUX -D_GNU_SOURCE -DUSE_LIBV4L2 -fPIC -shared
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CFLAGS += -mtune=cortex-a8 -march=armv7-a -ffast-math -DLINUX -D_GNU_SOURCE -DUSE_LIBV4L2 -fPIC -shared
LDLIBS += -L./ -L/usr/arm-linux-gnueabi/lib
endif
LDFLAGS =
INC = -I ../pub/ -I ../cross
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = httpd.o output_http.o

.PHONY: all
all: output_http.so

.PHONY: All
All: clean output_http.so

output_http.so: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o output_http.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	$(RM) *.o output_http.so

