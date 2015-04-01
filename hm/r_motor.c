#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "pub.h"
#include "gpio.h"
#include "r_usonic.h"

static unsigned int *left_speed = NULL;
static unsigned int *right_speed = NULL;

const int motor_pin[][6] = {
    /* connector, PIN */
    /* dir ctrl, dir ctrl, speed ctrl  */
    {9, 11, 9, 12, 8, 13},        /* Left DC MOTOR control & dir */
    {9, 13, 9, 14, 8, 19},        /* Right DC MOTOR control & dir */
};

const int motor_decoder_pin[2][2] = {
    {8, 22},
    {8, 23},
};

void motor_regist()
{
    regist_gpio(motor_pin[0][0], motor_pin[0][1], DIR_OUT);
    regist_gpio(motor_pin[0][2], motor_pin[0][3], DIR_OUT);
    regist_gpio(motor_pin[0][4], motor_pin[0][5], DIR_OUT);

    regist_gpio(motor_pin[1][0], motor_pin[1][1], DIR_OUT);
    regist_gpio(motor_pin[1][2], motor_pin[1][3], DIR_OUT);
    regist_gpio(motor_pin[1][4], motor_pin[1][5], DIR_OUT);

//    regist_gpio(motor_decoder_pin[0][0], motor_decoder_pin[0][1], DIR_OUT);
//    regist_gpio(motor_decoder_pin[1][0], motor_decoder_pin[1][1], DIR_OUT);
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
    right_speed = (unsigned int*) (pru_mem)+1;

    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem);
    if(pru_mem == NULL)
    {
        printf("ERR, %d\n", __LINE__);
        return -1;
    }

    left_speed = (unsigned int*) (pru_mem)+1;

int i;
for(i=0;i<32;i++) {
left_speed[i]=0;
right_speed[i]=0;
}

    /* Execute example on PRU */
    prussdrv_exec_program (0, "./hm_pru0.bin");
    prussdrv_exec_program (1, "./hm_pru1.bin");
    
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

int my_motor_speed(void *data)
{
    int i;
    printf("get_motor_left_speed...\n");
    for(i = 0;i < 32;i++)
    {
        if((left_speed != NULL) && (left_speed[i] != 0))
        {
            printf("left[%d]: 0x%x  \n", i, left_speed[i]);
        }
        if((right_speed != NULL) && (right_speed[i] != 0))
        {
            printf("right[%d]: 0x%x  \n", i, right_speed[i]);
        }
    }
    printf("\n");

    return 0;
}

