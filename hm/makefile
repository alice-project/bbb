include .config

CROSS_COMPILE =
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CROSS_COMPILE = arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -O -g2 
LDLIBS = -O -lrt -lpthread -ljpeg -liw
ifeq ($(CONFIG_ENABLE_CROSS_COMPILE),y)
CFLAGS += -mtune=cortex-a8 -march=armv7-a -ffast-math
LDLIBS += -L./ -L/usr/arm-linux-gnueabi/lib
endif
LDFLAGS =
INC = -I ../pub/
LN = $(CC)
RM = rm -f
MV = mv
AR = $(CROSS_COMPILE)ar

OBJS = main.o jpeg_utils.o r_commu.o  r_message.o r_timer.o

ifeq ($(CONFIG_ENABLE_GPIO_MMAP),y)
OBJS += gpio_mmap.o
endif
ifeq ($(CONFIG_ENABLE_GPIO_DTS),y)
OBJS += gpio_dts.o
endif
ifeq ($(CONFIG_ENABLE_PWM_DTS),y)
OBJS += pwm_dts.o
endif
ifeq ($(CONFIG_ENABLE_PWM_PRUSS),y)
OBJS += pwm_pru.o
LDLIBS += -lprussdrv
endif
ifeq ($(CONFIG_ENABLE_I2C_FUNC),y)
OBJS += i2c_func.o
endif
ifeq ($(CONFIG_ENABLE_MPU6050),y)
OBJS += mpu_dmp_motion_drv.o ml.o r_mpu6050.o  dynctrl.o
endif
ifeq ($(CONFIG_ENABLE_MOTOR),y)
OBJS += r_motor.o
endif
ifeq ($(CONFIG_ENABLE_USONIC),y)
OBJS += r_usonic.o
endif
ifeq ($(CONFIG_ENABLE_LED),y)
OBJS += r_led.o
endif
ifeq ($(CONFIG_ENABLE_SERVO),y)
OBJS += r_servo.o
endif
ifeq ($(CONFIG_ENABLE_VIDEO),y)
OBJS += input_uvc.o r_video.o v4l2uvc.o
endif
ifeq ($(CONFIG_ENABLE_WIFI_LOC),y)
OBJS += wifi_loc.o
endif

.PHONY: all
all: robot

.PHONY: All
All: clean robot

robot: $(OBJS)
	$(LN) $(LDFLAGS) $(CFLAGS) -o robot $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: config
config:
	./gen-config-h.sh ./.config > hm_config.h

.PHONY: clean
clean:
	-$(RM) *.o robot

