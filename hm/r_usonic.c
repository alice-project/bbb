#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "gpio.h"
#include "r_usonic.h"

#define MAX_USONIC_DETECT_TM  0x1ffff0

static int usonic_state[MAX_USONIC];
static struct timeval tm_start[MAX_USONIC];
static unsigned int *distance = NULL;

const int usonic_pin[MAX_USONIC][4] = {
    /* connector, trigger, conector, echo */
    {8,  7, 9, 28},
    {8,  8, 9, 29},
    {8,  9, 9, 30},
    {8, 10, 9, 31},
};

int usonic_regist()
{
    regist_gpio(usonic_pin[0][0], usonic_pin[0][1], DIR_OUT);
//    regist_gpio(usonic_pin[0][2], usonic_pin[0][3], DIR_IN);
    regist_gpio(usonic_pin[1][0], usonic_pin[1][1], DIR_OUT);
//    regist_gpio(usonic_pin[1][2], usonic_pin[1][3], DIR_IN);
    regist_gpio(usonic_pin[2][0], usonic_pin[2][1], DIR_OUT);
//    regist_gpio(usonic_pin[2][2], usonic_pin[2][3], DIR_IN);
    regist_gpio(usonic_pin[3][0], usonic_pin[3][1], DIR_OUT);
//    regist_gpio(usonic_pin[3][2], usonic_pin[3][3], DIR_IN);

    return 0;
}


static void *pru_mem = NULL;
static int PruInit ( unsigned short pruNum )
{
    //Initialize pointer to PRU data memory
    if (pruNum == 0)
    {
        prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru_mem);
    }
    else if (pruNum == 1)
    {
        prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru_mem);
    }

    if(pru_mem == NULL)
    {
        return -1;
    }

    distance = (unsigned int*) (pru_mem) + 1;

    // Flush the values in the PRU data memory locations
    distance[0] = 0x00;
    distance[1] = 0x00;
    distance[2] = 0x00;
    distance[3] = 0x00;

    return(0);
}

int usonic_init()
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    /* Initialize the PRU */
    prussdrv_init();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    PruInit(0);

    /* Execute example on PRU */
    prussdrv_exec_program (0, "./hm_pru0.bin");
    return(0);
    
}


int usonic_sensor_scan(void *data)
{
    struct timeval tm_current;
    unsigned long ms_diff;
    int i;

    for(i = 0;i < MAX_USONIC;i++)
    {
        if(distance != NULL)
            printf("USONIC%d: %d\n",i , distance[i]);
else
printf("distance[] is NULL\n");
    }

    return 0;
}

