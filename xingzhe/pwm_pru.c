#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "pwm.h"

static void *pru_mem;
struct pwm_param *pwm_param;

void pwm_pru_setup(int id, unsigned int period, unsigned int duty)
{
    pwm_param[id].occpy_cycle = period - duty;
    pwm_param[id].idle_cycle = duty;
}

int pwm_pru_set_duty(int id, unsigned int duty)
{
    unsigned int period;
    period = pwm_param[id].occpy_cycle + pwm_param[id].idle_cycle;
    pwm_param[id].occpy_cycle = period - duty;
    pwm_param[id].idle_cycle = duty;
}

int pwm_pru_init()
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    /* Initialize the PRU */
    prussdrv_init ();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    //Initialize pointer to PRU data memory
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru_mem);
    pwm_param = (struct pwm_param *) pru_mem;

    pwm_pru_setup(0, 20000000, 15000000);
    pwm_pru_setup(1, 20000000, 15000000);
    pwm_pru_setup(2, 20000000, 15000000);
    pwm_pru_setup(3, 20000000, 15000000);
    
    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (0, "./pwm_pru.bin");

    return 0;
}

