#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "gpio.h"
#include "r_usonic.h"

#define MAX_USONIC_DETECT_TM  0xffff0

static int usonic_state[MAX_USONIC];
static struct timeval tm_start[MAX_USONIC];
int distance[MAX_USONIC];

const int usonic_pin[MAX_USONIC][4] = {
    /* connector, trigger, conector, echo */
    {8, 11, 9, 15},
    {9, 17, 9, 18},
    {9, 22, 9, 23},
    {9, 24, 9, 25},
};

void usonic_regist()
{
    regist_gpio(usonic_pin[0][0], usonic_pin[0][1], DIR_OUT);
    regist_gpio(usonic_pin[0][2], usonic_pin[0][3], DIR_IN);
    regist_gpio(usonic_pin[1][0], usonic_pin[1][1], DIR_OUT);
    regist_gpio(usonic_pin[1][2], usonic_pin[1][3], DIR_IN);
    regist_gpio(usonic_pin[2][0], usonic_pin[2][1], DIR_OUT);
    regist_gpio(usonic_pin[2][2], usonic_pin[2][3], DIR_IN);
    regist_gpio(usonic_pin[3][0], usonic_pin[3][1], DIR_OUT);
    regist_gpio(usonic_pin[3][2], usonic_pin[3][3], DIR_IN);
}

void start_usonic_detect(int i)
{
  #ifdef DEBUG
    printf("start detecting ......\n");
  #endif
    set_pin_high(usonic_pin[i][0], usonic_pin[i][1]);
    gettimeofday(&tm_start[i], NULL);
    usonic_state[i] = USONIC_BUSY;
}

void stop_usonic_detect(int i)
{
  #ifdef DEBUG
    printf("stop detecting!\n");
  #endif

    set_pin_low(usonic_pin[i][0], usonic_pin[i][1]);
    usonic_state[i] = USONIC_IDLE;
}

void *usonic_sensor_scan(void *data)
{
    struct timeval tm_current;
    unsigned long ms_diff;
    int i;

  #ifdef DEBUG
    printf("usonic Scanning...\n");
  #endif

i=0;
printf("DIR=%d\n", gpio_get_dir(usonic_pin[i][0], usonic_pin[i][1]));
gpio_print_mode(usonic_pin[i][0], usonic_pin[i][1]);
printf("DIR=%d\n", gpio_get_dir(usonic_pin[i][2], usonic_pin[i][3]));
gpio_print_mode(usonic_pin[i][2], usonic_pin[i][3]);

#if 1
    for(;;)
    {
//        for(i = 0;i < MAX_USONIC;i++)
        {
            gettimeofday(&tm_current, NULL);
            ms_diff = (tm_current.tv_sec - tm_start[i].tv_sec) * 1000 + tm_current.tv_usec - tm_start[i].tv_usec;
            if(usonic_state[i] == USONIC_BUSY)
            {
                /* BUSY state SHOULD NOT BE trigger low */
                if(is_pin_low(usonic_pin[i][0], usonic_pin[i][1]))
                {
                    printf("state ERROR\n");
                    stop_usonic_detect(i);
printf("DIR=%d\n", gpio_get_dir(usonic_pin[i][0], usonic_pin[i][1]));
gpio_print_mode(usonic_pin[i][0], usonic_pin[i][1]);
                    continue;
                }

                if(ms_diff > MAX_USONIC_DETECT_TM)
                {
                    printf("DISTANCE is too far!\n");
printf("DIR=%d\n", gpio_get_dir(usonic_pin[i][2], usonic_pin[i][3]));
gpio_print_mode(usonic_pin[i][2], usonic_pin[i][3]);
                    stop_usonic_detect(i);
                    continue;
                }

                /* wait for echo */
                if(is_pin_high(usonic_pin[i][2], usonic_pin[i][3]))
                {
                    distance[i] = ms_diff/1700;  /* disance in CM */
                    printf("DISTANCE[%d] is %d, tm i %d\n", i, distance[i], ms_diff);
printf("DIR=%d\n", gpio_get_dir(usonic_pin[i][2], usonic_pin[i][3]));
gpio_print_mode(usonic_pin[i][2], usonic_pin[i][3]);
                    stop_usonic_detect(i);
                    continue;
                }
            }
            else
            {
                sleep(2);
                /* IDLE state SHOULD BE trigger low */
                if(is_pin_high(usonic_pin[i][0], usonic_pin[i][1]))
                {
                    printf("state ERROR\n");
                    stop_usonic_detect(i);
                    continue;
                }
                start_usonic_detect(i);
            }
        }
        usleep(10);
    }
#endif
    return NULL;
}

