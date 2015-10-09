#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "common.h"

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "pub.h"
#include "gpio.h"
#include "r_motor.h"

static u_int32 *left_speed_mem = NULL;
static u_int32 *right_speed_mem = NULL;

u_int32 left_speed_pulse=0;
u_int32 right_speed_pulse=0;
u_int32 left_speed_mm=0;
u_int32 right_speed_mm=0;
int32 speed_int;

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

u_int32 get_wheel_pwm(int dir)
{
	if(dir == LEFT_SIDE)
		return pwm_get_duty(motor_decoder_pin[0][0], motor_decoder_pin[0][1]);
	else
		return pwm_get_duty(motor_decoder_pin[1][0], motor_decoder_pin[1][1]);
}


static void *pru_mem0 = NULL;
static void *pru_mem1 = NULL;
static int motor_pru_init()
{
    u_int32 ret;
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

    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru_mem0);
    if(pru_mem0 == NULL)
    {
        printf("ERR, %d\n", __LINE__);
        return -1;
    }
    left_speed_mem = (u_int32*) (pru_mem0);

    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem1);
    if(pru_mem1 == NULL)
    {
        printf("ERR, %d\n", __LINE__);
        return -1;
    }

    right_speed_mem = (u_int32*) (pru_mem1);

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

    regist_cmd_cb(BA_MOTION_CMD, parser_motion_cmd);
}

int parser_motion_cmd(void * msg)
{
    if(msg == NULL)  return -1;

	struct s_base_motion *cmd = (struct s_base_motion *)msg;

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

/* using PID */
u_int32 speed_adjust(int speed_desired, int speed_current, int speed_int, u_int32 pwm)
{
	u_int32 new_pwm;
	float K, M;

	new_pwm = pwm+K*(speed_desired-speed_current)+M*speed_int;
	return new_pwm;
}

void *detect_speed(void *data)
{
    u_int32 cnt=0;
	speed_int = 0;

	u_int32 new_pwm;
	*left_speed_mem=0;
	*right_speed_mem=0;        
    for(;;)
    {
		memcpy(&left_speed_pulse, left_speed_mem, sizeof(int));
		left_speed_mm = (u_int32)((double)left_speed_pulse*(double)WHEEL_RADIUS*2*3.14/200000000.0)*10;
		
		memcpy(&right_speed_pulse, right_speed_mem, sizeof(int));
		right_speed_mm = (u_int32)((double)right_speed_pulse*(double)WHEEL_RADIUS*2.0*3.14/200000000.0)*10;

		/* intergal control */
		speed_int += (left_speed_pulse - right_speed_pulse)*0.1;

		cnt++;
		if((cnt%10)==0)
		{
			new_pwm=speed_adjust(left_speed_pulse, right_speed_pulse, speed_int, get_wheel_pwm(LEFT_SIDE));

			cnt=0;
		}
		*left_speed_mem=0;
		*right_speed_mem=0;        
		usleep(100000);
    }

    return NULL;
}


