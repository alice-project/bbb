#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "pub.h"
#include "gpio.h"
#include "r_motor.h"

static unsigned int *left_speed = NULL;
static unsigned int *left_flag = NULL;

static unsigned int *right_speed = NULL;
static unsigned int *right_flag = NULL;

const int motor_pin[][6] = {
    /* connector, PIN */
    /* dir ctrl, dir ctrl, speed ctrl  */
    {9, 11, 9, 12, 8, 13},        /* Left DC MOTOR control & dir */
    {9, 13, 9, 14, 8, 19},        /* Right DC MOTOR control & dir */
};

const int motor_decoder_pin[2][2] = {
    {8, 16},
    {8, 27},
};

void motor_regist()
{
    regist_gpio(motor_pin[0][0], motor_pin[0][1], DIR_OUT);
    regist_gpio(motor_pin[0][2], motor_pin[0][3], DIR_OUT);
    regist_gpio(motor_pin[0][4], motor_pin[0][5], DIR_OUT);

    regist_gpio(motor_pin[1][0], motor_pin[1][1], DIR_OUT);
    regist_gpio(motor_pin[1][2], motor_pin[1][3], DIR_OUT);
    regist_gpio(motor_pin[1][4], motor_pin[1][5], DIR_OUT);

    regist_gpio(motor_decoder_pin[0][0], motor_decoder_pin[0][1], DIR_IN);
    regist_gpio(motor_decoder_pin[1][0], motor_decoder_pin[1][1], DIR_IN);
}

int start_motor(int m)
{
    if(m==0)
    {
        pwm_run(motor_pin[0][4],motor_pin[0][5]);
        pwm_set_polarity(motor_pin[0][4], motor_pin[0][5], 1);
    } else {
        pwm_run(motor_pin[1][4],motor_pin[1][5]);
        pwm_set_polarity(motor_pin[1][4], motor_pin[1][5], 1);
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


static void *pru_mem = NULL;
static int motor_pru_init()
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    /* Initialize the PRU */
    prussdrv_init();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open 0 failed\n");
        return (ret);
    }

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_1);
    if (ret)
    {
        printf("prussdrv_open open 0 failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru_mem);
    if(pru_mem == NULL)
    {
        printf("ERR, %d\n", __LINE__);
        return -1;
    }
    right_flag  = (unsigned int*) (pru_mem);
    right_speed = (unsigned int*) (pru_mem)+1;

    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem);
    if(pru_mem == NULL)
    {
        printf("ERR, %d\n", __LINE__);
        return -1;
    }

    left_flag  = (unsigned int*) (pru_mem);
    left_speed = (unsigned int*) (pru_mem)+1;

    /* Execute example on PRU */
    prussdrv_exec_program (0, "./hm_pru0.bin");
    prussdrv_exec_program (1, "./hm_pru1.bin");

*left_flag=0;
*left_speed=1;

*right_flag=2;
*right_speed=3;

    return(0);
}

void motor_init()
{
    printf("motor init!\n");
    motor_pru_init();

    stop_chassis();

    set_pin_high(motor_pin[0][0], motor_pin[0][1]);
    set_pin_low(motor_pin[0][2], motor_pin[0][3]);
    pwm_set_polarity(motor_pin[0][4], motor_pin[0][5], 1);

    set_pin_high(motor_pin[1][0], motor_pin[1][1]);
    set_pin_low(motor_pin[1][2], motor_pin[1][3]);
    pwm_set_polarity(motor_pin[1][4], motor_pin[1][5], 1);

    regist_cmd_cb(BA_MOTION_CMD, parser_motion_cmd);
}

int parser_motion_cmd(struct s_base_motion *cmd)
{
    if(cmd == NULL)  return -1;

    if(cmd->left_action == START_ACTION)
    {
        start_chassis();
        if(cmd->left_data == POSITIVE_DIR)
        {
            set_pin_high(motor_pin[0][0], motor_pin[0][1]);
            set_pin_low(motor_pin[0][2], motor_pin[0][3]);
        }
        else if(cmd->left_data == NEGATIVE_DIR)
        {
            set_pin_low(motor_pin[0][0], motor_pin[0][1]);
            set_pin_high(motor_pin[0][2], motor_pin[0][3]);
        }
    }
    if(cmd->left_action == SET_DUTY_ACTION)
    {
        if(pwm_set_duty(motor_pin[0][4], motor_pin[0][5], cmd->left_data) < 0)
            printf("FAILED\n");
    }
    if(cmd->left_action == STOP_ACTION)
    {
        stop_motor(0);
    }

    if(cmd->right_action == START_ACTION)
    {
        start_chassis();
        if(cmd->right_data == POSITIVE_DIR)
        {
            set_pin_high(motor_pin[1][0], motor_pin[1][1]);
            set_pin_low(motor_pin[1][2], motor_pin[1][3]);
        }
        else if(cmd->right_data == NEGATIVE_DIR)
        {
            set_pin_low(motor_pin[1][0], motor_pin[1][1]);
            set_pin_high(motor_pin[1][2], motor_pin[1][3]);
        }
    }
    if(cmd->right_action == SET_DUTY_ACTION)
    {
        if(pwm_set_duty(motor_pin[1][4], motor_pin[1][5], cmd->right_data) < 0)
            printf("FAILED!\n");
    }
    if(cmd->right_action == STOP_ACTION)
    {
        stop_motor(1);
    }

    return 0;
}

void *detect_left_speed(void *data)
{
    u_int32 cnt=0;
    printf("get_motor_left_speed...\n");
    for(;;)
    {
		*left_flag=0;
		*left_speed=0;
        sleep(1);
        printf("left flag=0x%x(%d/s),SPEED=0x%x\n", *left_flag, *left_flag/200000000, *left_speed);
    }
    printf("\n");

    return NULL;
}

void *detect_right_speed(void *data)
{
    u_int32 cnt=0;
    printf("get_motor_right_speed...\n");
    for(;;)
    {
		*right_flag=0;
		*right_speed=0;
        sleep(1);
        printf("right flag=0x%x(%d/s),SPEED=0x%x\n", *right_flag, *right_flag/200000000, *right_speed);
    }
    printf("\n");

    return NULL;
}

