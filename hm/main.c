#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "pub.h"
#include "common.h"
#include "gpio.h"
#include "r_event.h"
#include "r_message.h"
#include "r_timer.h"
#include "r_usonic.h"
#include "r_led.h"
#include "r_commu.h"

struct s_hm_state m_state;

int system_init() 
{
    m_state.m_moving = 0;
    m_state.m_control = MANUAL_MODE;
    m_state.m_camera = 0;
    m_state.m_usonic0 = 0;
    m_state.m_usonic1 = 0;
    m_state.m_usonic2 = 0;
    m_state.m_usonic3 = 0;
    m_state.m_left_pwm = 0;
    m_state.m_right_pwm = 0;
    
    if(r_message_init() < 0)
        return -1;

    if(r_timer_init() < 0)
        return -1;

//    led_regist();

//    usonic_regist();
    
//    if(gpio_init() < -1)
//        return -1;

    return 0;
}

int main(int argc, char *argv[])
{
    message_node *node;
    struct r_msg *msg;

    pthread_t tm_thread;
    pthread_t sonic_thread;
    pthread_t commu_thread;

    if(system_init() < 0)
        return -1;

//    set_timer(0, R_TIMER_LOOP, 1000, led_blink, NULL);
    pthread_create(&tm_thread, NULL, &timer_scan_thread, NULL);
//    pthread_create(&sonic_thread, NULL, &usonic_sensor_scan, NULL);
    pthread_create(&commu_thread, NULL, &commu_proc, NULL);

    while(1)
    {
        node = r_get_message();
        if(node == NULL)
        {
            usleep(10);
            continue;
        }
        
        msg = (struct r_msg *)node->data;
        if(msg->msg_id == R_MSG_INVALID)
        {
            usleep(10);
            continue;
        }

        switch(msg->msg_id)
        {
            case R_MSG_SET_DCMOTOR_SPEED:
                break;

            default:
                break;
        }
    }

    pthread_join(tm_thread, NULL);
    pthread_join(sonic_thread, NULL);
    pthread_join(commu_thread, NULL);

    gpio_exit();

    return 0;
}
