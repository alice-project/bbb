#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#ifndef __GPIO_H__
#define __GPIO_H__

enum {
    GPIO_MODE_0 = 0,
    GPIO_MODE_1,
    GPIO_MODE_2,
    GPIO_MODE_3,
    GPIO_MODE_4,
    GPIO_MODE_5,
    GPIO_MODE_6,
    GPIO_MODE_7,
};

enum {
    DIR_OUT = 0,
    DIR_IN,
};

#define CONTROL_MODULE 0x44e10000
#define CONTROL_LEN 0xA00
#define GPIO0_BASE 0x44e07000
#define GPIO1_BASE 0x4804c000
#define GPIO2_BASE 0x481ac000
#define GPIO3_BASE 0x481ae000
#define GPIOX_LEN  0x198

#define GPIO_REVISION_OFFSET            0x0
#define GPIO_SYSCONFIG_OFFSET           0x10
#define GPIO_EOI_OFFSET                 0x20
#define GPIO_IRQSTATUS_RAW_0_OFFSET     0x24
#define GPIO_IRQSTATUS_RAW_1_OFFSET     0x28
#define GPIO_IRQSTATUS_0_OFFSET         0x2c
#define GPIO_IRQSTATUS_1_OFFSET         0x30
#define GPIO_IRQSTATUS_SET_0_OFFSET     0x34
#define GPIO_IRQSTATUS_SET_1_OFFSET     0x38
#define GPIO_IRQSTATUS_CLR_0_OFFSET     0x3c
#define GPIO_IRQSTATUS_CLR_1_OFFSET     0x40
#define GPIO_IRQWAKEN_0_OFFSET          0x44
#define GPIO_IRQWAKEN_1_OFFSET          0x48
#define GPIO_SYSSTATUS_OFFSET           0x114
#define GPIO_CTRL_OFFSET                0x130
#define GPIO_OE_OFFSET         	        0x134
#define GPIO_DATAIN_OFFSET     	        0x138
#define GPIO_DATAOUT_OFFSET    	        0x13c
#define GPIO_LEVELDETECT0_OFFSET        0x140
#define GPIO_LEVELDETECT1_OFFSET        0x144
#define GPIO_RISINGDETECT_OFFSET        0x148
#define GPIO_FALLINGDETECT_OFFSET       0x14c
#define GPIO_DEBOUNCENABLE_OFFSET       0x150
#define GPIO_DEBOUNCINGTIME_OFFSET      0x154
#define GPIO_CLEARDATAOUT_OFFSET        0x190
#define GPIO_SETDATAOUT_OFFSET          0x194

#define BIT_IS_SET(x,b) (((x)>>(b)) & 0x1)

enum GPIO_IRQ_MODE {
    GPIO_IRQ_LOW_LEVEL = 0,
    GPIO_IRQ_HIGH_LEVEL,
    GPIO_IRQ_RISING_EDGE,
    GPIO_IRQ_FALLING_EDGE,
};

int gpio_init();
int gpio_exit();
int gpio_set_dir(int connector, int pin, int dir);
int set_pin_high(int connector, int pin);
int set_pin_low(int connector, int pin);
int is_pin_high(int connector, int pin);
int is_pin_low(int connector, int pin);
void regist_gpio(int connector, int pin, int dir);
int gpio_get_dir(int connector, int pin);
void gpio_print_mode(int connector, int pin);
int set_pin_irq_mode(int connector, int pin, enum GPIO_IRQ_MODE mode);

#endif
