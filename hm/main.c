#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "pub.h"
#include "common.h"
#include "gpio.h"
#include "r_event.h"
#include "r_message.h"
#include "r_timer.h"
#include "r_usonic.h"
#include "r_motor.h"
#include "r_led.h"
#include "r_commu.h"
#include "r_test.h"
#include "r_servo.h"
#include "r_mpu6050.h"

struct s_hm_state m_state;
static int fd_wtdog=-1;

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

    led_regist();

//    usonic_regist();
    servo_regist();
    motor_regist();

    if(gpio_init() < 0)
        return -1;

    if(usonic_init() < 0)
    {
        printf("USONIC INIT failed!\n");
        return -1;
    }
    printf("USONIC INIT FIN!\n");

    motor_init();
    printf("MOTOR INIT FIN!\n");

    servo_init();

    if(video_init(0) < 0)
    {
        printf("VIDEO INIT failed!\n");
        return -1;
    }
    printf("VIDEO INIT!\n");
    
    fd_wtdog = open("/dev/watchdog", O_WRONLY);  
    if(fd_wtdog == -1) {  
        int err = errno;  
        printf("\n!!! FAILED to open /dev/watchdog, errno: %d, %s\n", err, strerror(err));  
        syslog(LOG_WARNING, "FAILED to open /dev/watchdog, errno: %d, %s", err, strerror(err));  
    }
    
    return 0;
}

static void feed_wtdog()
{
    if(fd_wtdog >= 0) {  
        if(write(fd_wtdog, "1", 1) != 1) {  
            puts("\n!!! FAILED feeding watchdog");  
            syslog(LOG_WARNING, "FAILED feeding watchdog");  
        }  
    }
}

int main(int argc, char *argv[])
{
    message_node *node;
    struct r_msg *msg;

    pthread_t tm_thread;
    pthread_t commu_thread;

    if(system_init() < 0)
        return -1;

    video_start(0);

    set_timer(0, R_TIMER_LOOP, 10, led_shining, NULL);
//    set_timer(R_MSG_TEST, R_TIMER_LOOP, 5000, hm_test, NULL);
//    set_timer(R_MSG_MOTOR_SPEED, R_TIMER_LOOP, 4000, my_motor_speed, NULL);
//    set_timer(R_MSG_USONIC_DETECT, R_TIMER_LOOP, 1000, usonic_detect, NULL);
//    set_timer(R_MSG_MPU6050_DETECT, R_TIMER_LOOP, 1000, mpu6050_detect, NULL);
    set_timer(R_MSG_SERVO_0, R_TIMER_LOOP, 1000, servo_0_rotating, 0);

    pthread_create(&tm_thread, NULL, &timer_scan_thread, NULL);
    pthread_create(&commu_thread, NULL, &commu_proc, NULL);

    while(1)
    {
        feed_wtdog();
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
        usleep(100);
    }

    pthread_join(tm_thread, NULL);
    pthread_join(commu_thread, NULL);

    gpio_exit();

    return 0;
}
