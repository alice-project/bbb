CROSS_COMPILE = arm-linux-gnueabihf-
@CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
@CFLAGS = -O -g2 -DDEBUG -DGPIO_USING_DTS
CFLAGS = -O -g2 -DDEBUG
LDLIBS = -lrt -L./ -lpthread -lprussdrv
LDFLAGS =
INC = -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = gpio_mmap.o gpio_dts.o pwm_dts.o r_timer.o  r_motor.o r_message.o r_usonic.o r_led.o r_commu.o r_test.o r_servo.o main.o

all: robot

All: clean robot

robot: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o robot $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	-$(RM) *.o robot
