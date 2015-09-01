#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "pub.h"
#include "gpio.h"
#include "pwm.h"

#define SERVO_STD_PERIOD     20000000
#define SERVO_MIN_LEFT       17500000
#define SERVO_MAX_RIGHT      19500000
#define SERVO_MIDDLE_DUTY    18500000
#define SERVO_MAX_ANGLE      180
#define SERVO_ANGLE_TO_DUTY(g)    (SERVO_MIDDLE_DUTY+(g)/SERVO_MAX_ANGLE*(SERVO_MAX_RIGHT-SERVO_MIN_LEFT))
#define SERVO_DUTY_TO_ANGLE(d)    (((d)-SERVO_MIDDLE_DUTY)*SERVO_MAX_ANGLE/(SERVO_MAX_RIGHT-SERVO_MIN_LEFT))

#define SERVO2_MAX_ANGLES    SERVO_DUTY_TO_ANGLE(19500000)
#define SERVO2_MIN_ANGLE     SERVO_DUTY_TO_ANGLE(19200000)

#define SERVOROTATE_STEP    10.0

#define SERVO2_ROTATE_STEP    2.0

#define ROTATE_POSITIVE     0
#define ROTATE_NEGATIVE     1

int camera_auto_rotating = 1;

const int servo_pin[][2] = {
    /* connector, PIN */
    {9, 21},
    {9, 22},
};

static int servo_rotate_direc = 0;

void servo_regist()
{
    regist_gpio(servo_pin[0][0], servo_pin[0][1], DIR_OUT);
    regist_gpio(servo_pin[1][0], servo_pin[1][1], DIR_OUT);
}

void servo_rotate_to(int id, double angle)
{
    if(id > sizeof(servo_pin)/sizeof(servo_pin[0]))
        return;

    /* valid angle is [-90, 90] */
    if((angle < -90.0) || (angle > 90.0))
        return;

    pwm_set_duty_cycle(servo_pin[id][0], servo_pin[id][1], SERVO_ANGLE_TO_DUTY(angle));
}

void servo_init()
{
    int i;
    for(i=0;i<sizeof(servo_pin)/sizeof(servo_pin[0]);i++)
    {
        pwm_stop(servo_pin[i][0], servo_pin[i][1]);
        pwm_set_period(servo_pin[i][0], servo_pin[i][1], SERVO_STD_PERIOD);
        pwm_set_polarity(servo_pin[i][0], servo_pin[i][1], 1);
        pwm_set_duty_cycle(servo_pin[i][0], servo_pin[i][1], SERVO_ANGLE_TO_DUTY(0));  /* keep in the middle */
    }
    start_servo();
}

int start_servo()
{
    int i;
    for(i=0;i<sizeof(servo_pin)/sizeof(servo_pin[0]);i++)
    {
        pwm_run(servo_pin[i][0], servo_pin[i][1]);
    }
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
    period = pwm_get_period(servo_pin[0][0], servo_pin[0][1]);
    duty_cycle = pwm_get_duty(servo_pin[0][0], servo_pin[0][1]);
    angle = SERVO_DUTY_TO_ANGLE(duty_cycle);

    if(angle > 90.0)
    {
        servo_rotate_direc = ROTATE_NEGATIVE;
    }

    if(angle < -90.0)
    {
        servo_rotate_direc = ROTATE_POSITIVE;
    }

    if(servo_rotate_direc == ROTATE_POSITIVE) {
        servo_rotate_to(0, angle);
    } else {
        servo_rotate_to(0, angle);
    }
}

int regular_servo_rotating(void *data)
{
    static int servo1_dir=0;
    static int servo2_dir=0;
    int duty_cycle1, duty_cycle2;
    double angle1, angle2;

    duty_cycle1 = pwm_get_duty(servo_pin[0][0], servo_pin[0][1]);
    duty_cycle2 = pwm_get_duty(servo_pin[1][0], servo_pin[1][1]);
    if(duty_cycle1 >= SERVO_STD_PERIOD-SERVO_MAX_RIGHT) {
        if(duty_cycle2 >= SERVO_STD_PERIOD-SERVO_MAX_RIGHT) {
            servo1_dir = -1;
            servo2_dir = -1;
            goto SERVO_ROTATE;
        } else {
            servo1_dir = -1;
            servo2_dir = 0;
            goto SERVO_ROTATE;
        }
    } else {
        if(duty_cycle2 >= SERVO_STD_PERIOD-SERVO_MAX_RIGHT) {
            servo1_dir = 0;
            servo2_dir = -1;
            goto SERVO_ROTATE;
        } else {
            servo1_dir = 0;
            servo2_dir = 0;
            goto SERVO_ROTATE;
        }
    }

SERVO_ROTATE:
    angle1 = SERVO_DUTY_TO_ANGLE(duty_cycle1);
    angle2 = SERVO_DUTY_TO_ANGLE(duty_cycle2);

    if((servo1_dir==0)&&(servo2_dir==0)) {
        servo_rotate_to(0, angle1+SERVOROTATE_STEP);
    } else if((servo1_dir==0)&&(servo2_dir==-1)) {
        servo_rotate_to(1, angle2-SERVOROTATE_STEP);
    } else if((servo1_dir==-1)&&(servo2_dir==0)) {
        servo_rotate_to(1, angle2+SERVOROTATE_STEP);
    } else if((servo1_dir==-1)&&(servo2_dir==-1)) {
        servo_rotate_to(0, angle1-SERVOROTATE_STEP);
    }
}

void *servo_proc(void *data)
{
    static int servo1_dir=0;
    static int servo2_dir=0;
    int duty_cycle1, duty_cycle2;
    double angle1, angle2;

    angle1 = 0.0;
    angle2 = SERVO2_MIN_ANGLE;
    while(1)
    {
        if(camera_auto_rotating)
        {
            sleep(1);
            continue;
        }

        if(servo1_dir==0) {
            if(angle1 < SERVO_MAX_ANGLE-SERVOROTATE_STEP) {
                angle1 += SERVOROTATE_STEP;
                servo_rotate_to(0, angle1);
            } else {
                servo1_dir = -1;
                if(servo2_dir==0) {
                    angle2 += SERVO2_ROTATE_STEP;
                    servo_rotate_to(1, angle2);
                    if(angle2 > SERVO_MAX_ANGLE-SERVO2_ROTATE_STEP)
                        servo2_dir = -1;
                } else {
                    angle2 -= SERVOROTATE_STEP;
                    servo_rotate_to(1, angle2);
                    if(angle2 < SERVO2_MIN_ANGLE-SERVO2_ROTATE_STEP)
                        servo2_dir = 0;
                }
            }
        } else {
            if(angle1 > SERVOROTATE_STEP) {
                angle1 -= SERVOROTATE_STEP;
                servo_rotate_to(0, angle1);
            } else {
                servo1_dir = 0;
                if(servo2_dir==0) {
                    angle2 += SERVO2_ROTATE_STEP;
                    servo_rotate_to(1, angle2);
                    if(angle2 > SERVO_MAX_ANGLE-SERVO2_ROTATE_STEP)
                        servo2_dir = -1;
                } else {
                    angle2 -= SERVO2_ROTATE_STEP;
                    servo_rotate_to(1, angle2);
                    if(angle2 < SERVO2_MIN_ANGLE-SERVO2_ROTATE_STEP)
                        servo2_dir = 0;
                }
            }
        }
        usleep(100000);
    }

    return NULL;
}

void set_camera_rotate(int state)
{
    if(state)
        camera_auto_rotating = 1;
    else
        camera_auto_rotating = 0;
}


