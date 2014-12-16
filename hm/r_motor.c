#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "pub.h"
#include "gpio.h"
#include "r_usonic.h"

const int motor_pin[][6] = {
    /* connector, PIN */
    /* dir ctrl, dir ctrl, speed ctrl  */
    {9, 11, 9, 12, 8, 13},        /* Left DC MOTOR control & dir */
    {9, 13, 9, 14, 8, 19},        /* Right DC MOTOR control & dir */
};

void motor_regist()
{
    regist_gpio(motor_pin[0][0], motor_pin[0][1], DIR_OUT);
    regist_gpio(motor_pin[0][2], motor_pin[0][3], DIR_OUT);
    regist_gpio(motor_pin[0][4], motor_pin[0][5], DIR_OUT);

    regist_gpio(motor_pin[1][0], motor_pin[1][1], DIR_OUT);
    regist_gpio(motor_pin[1][2], motor_pin[1][3], DIR_OUT);
    regist_gpio(motor_pin[1][4], motor_pin[1][5], DIR_OUT);
}

int start_motor(int m)
{
    if(m==0)
    {
        pwm_run(motor_pin[0][4],motor_pin[0][5]);
    } else {
        pwm_run(motor_pin[1][4],motor_pin[1][5]);
    }

    return 0;
}

int stop_motor(int m)
{
    if(m==0)
    {
        pwm_stop(motor_pin[0][4],motor_pin[0][5]);
    } else {
        pwm_stop(motor_pin[1][4],motor_pin[1][5]);
    }

    return 0;
}

int stop_chassis()
{
    stop_motor(0);
    stop_motor(1);
}

int start_chassis()
{
    start_motor(0);
    start_motor(1);
}

void motor_init()
{
    stop_chassis();
    set_pin_high(motor_pin[0][0], motor_pin[0][1]);
    set_pin_low(motor_pin[0][2], motor_pin[0][3]);
    pwm_set_polarity(motor_pin[0][4], motor_pin[0][5], 1);

    set_pin_high(motor_pin[1][0], motor_pin[1][1]);
    set_pin_low(motor_pin[1][2], motor_pin[1][3]);
    pwm_set_polarity(motor_pin[1][4], motor_pin[1][5], 1);
}

int parser_motion_cmd(struct s_base_motion *cmd)
{
    if(cmd == NULL)  return -1;

    if(cmd->action == START_ACTION) {
        printf("Start Motor!\n");
        start_chassis();
    } else {
        printf("Stop Motor!\n");
        stop_chassis();
    }

    return 0;
}

