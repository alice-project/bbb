@CROSS_COMPILE = arm-linux-gnueabihf-
CROSS_COMPILE =
@CROSS_COMPILE = /home/bbb/Downloads/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
@CFLAGS = -O -g2 -DDEBUG -DGPIO_USING_DTS
CFLAGS = -O -g2 -DDEBUG
LDLIBS = -lrt -lpthread 
LDFLAGS =
INC = -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = gpio_mmap.o gpio_dts.o r_timer.o r_message.o r_usonic.o r_led.o r_commu.o main.o

all: robot

All: clean robot

robot: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o robot $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o robot
