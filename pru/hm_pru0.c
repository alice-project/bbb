
/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

#include <stdio.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM 	0

#define AM33XX

/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int PruInit ( unsigned short pruNum );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Intertupt Service Routines                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pru_mem;
static unsigned int *distance;

/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
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
//    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: PRU Initializing ...\r\n");
    PruInit(PRU_NUM);

    /* Execute example on PRU */
    printf("\tINFO: Executing hm_pru0.\r\n");
    prussdrv_exec_program (PRU_NUM, "./hm_pru0.bin");

    /* Disable PRU and close memory mapping*/
//    prussdrv_pru_disable (PRU_NUM);
//    prussdrv_exit ();

    return(0);

}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

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
    distance = (unsigned int*) pru_mem;

    // Flush the values in the PRU data memory locations
    distance[0] = 0x00;
    distance[1] = 0x00;
    distance[2] = 0x00;
    distance[3] = 0x00;

    return(0);
}

static unsigned short read_distance ( unsigned int usonic )
{
  return distance[usonic];
}
