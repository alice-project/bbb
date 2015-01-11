#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "pub.h"
#include "gpio.h"
#include "r_usonic.h"

static unsigned int *speed = NULL;

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


static void *pru_mem = NULL;
static int Motor_PruInit()
{
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem);

    if(pru_mem == NULL)
    {
        return -1;
    }

    speed = (unsigned int*) (pru_mem)+1;

    // Flush the values in the PRU data memory locations
    speed[0] = 0x00;
    speed[1] = 0x00;

int i;
for(i=0;i<32;i++)
speed[i]=0;

    /* Execute example on PRU */
    prussdrv_exec_program (0, "./hm_pru1.bin");
    
    return(0);
}

static int motor_pruinit ()
{
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem);
    if(pru_mem == NULL)
    {
        return -1;
    }

    speed = (unsigned int*) (pru_mem) + 1;

    // Flush the values in the PRU data memory locations
    speed[0] = 0x00;
    speed[1] = 0x00;

int i;
for(i=0;i<32;i++)
speed[i]=0;

    return(0);
}



void motor_init()
{
    printf("motor init!\n");
    Motor_PruInit();

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

    if((cmd->left_action == START_ACTION) && (cmd->right_action == START_ACTION))
    {
        start_chassis();
        if(cmd->left_dir == POSITIVE_DIR)
        {
            set_pin_high(motor_pin[0][0], motor_pin[0][1]);
            set_pin_low(motor_pin[0][2], motor_pin[0][3]);
        }
        else if(cmd->left_dir == NEGATIVE_DIR)
        {
            set_pin_low(motor_pin[0][0], motor_pin[0][1]);
            set_pin_high(motor_pin[0][2], motor_pin[0][3]);
        }

        if(cmd->right_dir == POSITIVE_DIR)
        {
            set_pin_high(motor_pin[1][0], motor_pin[1][1]);
            set_pin_low(motor_pin[1][2], motor_pin[1][3]);
        }
        else if(cmd->right_dir == NEGATIVE_DIR)
        {
            set_pin_low(motor_pin[1][0], motor_pin[1][1]);
            set_pin_high(motor_pin[1][2], motor_pin[1][3]);
        }
    }
    else
    {
        stop_chassis();
    }
    return 0;
}

int my_motor_speed(void *data)
{
    int i;
/*    printf("get_motor_speed...\n");
    for(i = 0;i < 32;i++)
    {
        if(speed != NULL)
            printf("SPEED: %d  ", speed[i]);
        else
            printf("NULL  ");
    }
    printf("\n");
*/
    return 0;
}

