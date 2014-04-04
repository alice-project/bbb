#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "r_led.h"
#include "gpio.h"

//#define LED_CONNECTOR  9
//#define LED_PIN        11
#define LED_CONNECTOR  8
#define LED_PIN        11

int led_regist()
{
    regist_gpio(LED_CONNECTOR, LED_PIN, DIR_OUT);

    return 0;
}

int led_blink(void *d)
{
  #ifdef DEBUG
    printf("LED Blinking......\n");
  #endif
  	
    if(is_pin_high(LED_CONNECTOR, LED_PIN))
    {
        set_pin_low(LED_CONNECTOR, LED_PIN);
      #ifdef DEBUG
        printf("setting to low ");
        if(is_pin_low(LED_CONNECTOR, LED_PIN))
            printf("OK!\n");
        else
            printf("Failed!\n");
      #endif
    }
    else
    {
        set_pin_high(LED_CONNECTOR, LED_PIN);
      #ifdef DEBUG
        printf("setting to high ");
        if(is_pin_high(LED_CONNECTOR, LED_PIN))
            printf("OK!\n");
        else
            printf("Failed!\n");
      #endif
    }

    return 0;
}


