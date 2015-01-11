#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "pub.h"
#include "gpio.h"
#include "pwm.h"

#define SG90_STANDARD_PERIOD     20000000
#define SG90_ANGLE_TO_DUTY(g)    (1500000.0+((g)/90.0)*1000000.0)
#define SG90_DUTY_TO_ANGLE(d)    (((d)-1500000.0)/1000000.0*90.0)

#define SG90_ROTATE_STEP    5.0

#define ROTATE_POSITIVE     0
#define ROTATE_NEGATIVE     1

const int servo_pin[][6] = {
    /* connector, PIN */
    {9, 22},
};

static int servo_rotate_direc = 0;

void servo_regist()
{
    regist_gpio(servo_pin[0][0], servo_pin[0][1], DIR_OUT);
}

void servo_rotate_to(int connector, int pin, double angle)
{
    /* valid angle is [-90, 90] */
    if((angle < -90.0) || (angle > 90.0))
        return;

    pwm_set_duty_cycle(connector, pin, SG90_ANGLE_TO_DUTY(angle));
}

void servo_init()
{
    pwm_stop(servo_pin[0][0], servo_pin[0][1]);
    pwm_set_period(servo_pin[0][0], servo_pin[0][1], SG90_STANDARD_PERIOD);
    pwm_set_duty_cycle(servo_pin[0][0], servo_pin[0][1], SG90_ANGLE_TO_DUTY(0));  /* keep in the middle */
}

int start_servo()
{
    pwm_run(servo_pin[0][0], servo_pin[0][1]);
    return 0;
}

int parser_servo_cmd(struct s_base_motion *cmd)
{
    if(cmd == NULL)  return -1;

    if(cmd->left_action == START_ACTION) {
        printf("Start servo!\n");
        set_pin_high(servo_pin[0][0], servo_pin[0][1]);
    } else {
        printf("Stop servo!\n");
        set_pin_low(servo_pin[0][0], servo_pin[0][1]);
    }

    return 0;
}

int servo_0_rotating(void *data)
{
    double angle;
    int duty_cycle;
    int period;
printf("servo_0_rotating\n");
    period = pwm_get_period(servo_pin[0][0], servo_pin[0][1]);
    duty_cycle = pwm_get_duty(servo_pin[0][0], servo_pin[0][1]);
    angle = SG90_DUTY_TO_ANGLE(duty_cycle);

    if(angle > 90.0)
    {
        servo_rotate_direc = ROTATE_NEGATIVE;
    }

    if(angle < -90.0)
    {
        servo_rotate_direc = ROTATE_POSITIVE;
    }

    if(servo_rotate_direc == ROTATE_POSITIVE) {
        servo_rotate_to(servo_pin[0][0], servo_pin[0][1], angle+SG90_ROTATE_STEP);
    } else {
        servo_rotate_to(servo_pin[0][0], servo_pin[0][1], angle-SG90_ROTATE_STEP);
    }
}


