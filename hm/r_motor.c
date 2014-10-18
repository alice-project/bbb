#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "gpio.h"
#include "r_usonic.h"

const int motor_pin[][4] = {
    /* connector, PIN */
    {8, 39, 8, 40},        /* MOTOR control & dir */
    {8, 13, 8, 14},        /* MOTOR control & dir */
};

void motor_regist()
{
    regist_gpio(motor_pin[0][0], motor_pin[0][1], DIR_OUT);
    regist_gpio(motor_pin[0][2], motor_pin[0][3], DIR_OUT);
}

int start_motor()
{
    set_pin_high(motor_pin[0][0], motor_pin[0][1]);
//    set_pin_high(motor_pin[0][2], motor_pin[0][3]);
}

int stop_motor()
{
    set_pin_low(motor_pin[0][0], motor_pin[0][1]);
    set_pin_low(motor_pin[0][2], motor_pin[0][3]);
}


