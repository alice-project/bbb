#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "r_led.h"
#include "gpio.h"

#define R_LED_CONNECTOR  9
#define R_LED_PIN        15
#define G_LED_CONNECTOR  9
#define G_LED_PIN        16
#define B_LED_CONNECTOR  9
#define B_LED_PIN        23

static int shining_index=0;

static int led_switch=0;

const static int shining_rgb[][3] = {
//    {0, 0, 0},
    {0, 0, 1},
    {0, 1, 0},
    {1, 0, 0},
    {0, 1, 1},
    {1, 1, 0},
    {1, 0, 1},
    {1, 1, 1},
};

int led_regist()
{
    regist_gpio(R_LED_CONNECTOR, R_LED_PIN, DIR_OUT);
    regist_gpio(G_LED_CONNECTOR, G_LED_PIN, DIR_OUT);
    regist_gpio(B_LED_CONNECTOR, B_LED_PIN, DIR_OUT);

    return 0;
}

static void set_led(int connector, int pin, int value)
{
    if(value == 1) {
        set_pin_high(connector, pin);
    } else {
        set_pin_low(connector, pin);
    }
}

int set_led_on()
{
    led_switch = 1;
}

int set_led_off()
{
    led_switch = 0;
}

int led_shining(void *d)
{
    if(led_switch==0)
        return 0;

    if(shining_index > 7)
        shining_index = 0;

    set_led(R_LED_CONNECTOR, R_LED_PIN, shining_rgb[shining_index][0]);
    set_led(G_LED_CONNECTOR, G_LED_PIN, shining_rgb[shining_index][1]);
    set_led(B_LED_CONNECTOR, B_LED_PIN, shining_rgb[shining_index][2]);

    shining_index++;
    return 0;
}


