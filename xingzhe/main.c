#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

#include "xz_config.h"

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

extern MSG_HANDLER cmd_cb[64];

struct s_xz_state m_state;
static int fd_wtdog=-1;

extern void *wifi_location(void *data);

pthread_mutex_t mutex_i2c;

int system_init() 
{
    int i;
    for(i=0;i<64;i++)
        cmd_cb[i] = NULL;
    m_state.m_moving = 0;
    m_state.m_control = MANUAL_MODE;
    m_state.m_camera = 0;
    m_state.m_usonic0 = 0;
    m_state.m_usonic1 = 0;
    m_state.m_usonic2 = 0;
    m_state.m_usonic3 = 0;
    m_state.m_left_pwm = 0;
    m_state.m_right_pwm = 0;

	if(pthread_mutex_init(&mutex_i2c, NULL) != 0) {
        printf("could not initialize mutex variable\n");
        return -1;
    }
    
    if(r_message_init() < 0) {
		printf("r_message_init failed!\n");
        return -1;
	}

    if(r_timer_init() < 0) {
		printf("r_timer_init failed!\n");
        return -1;
	}

    #ifdef ENABLE_LED
    led_regist();
    #endif

    #ifdef ENABLE_USONIC
    usonic_regist();
    #endif

    #ifdef ENABLE_SERVO
    servo_regist();
    #endif

    #ifdef ENABLE_MOTOR
    motor_regist();
    #endif

    #ifdef ENABLE_MPU6050
    gyro_regist();
    #endif

	#if defined(ENABLE_GPIO_MMAP) || defined(ENABLE_GPIO_DTS)
    if(gpio_init() < 0) {
		printf("gpio_init failed!\n");
        return -1;
	}
	#endif

    #ifdef ENABLE_USONIC
    if(usonic_init() < 0) {
        printf("USONIC INIT failed!\n");
        return -1;
    }
    printf("USONIC INIT FIN!\n");
    #endif

    #ifdef ENABLE_MOTOR
    motor_init();
    printf("MOTOR INIT FIN!\n");
    #endif

    #ifdef ENABLE_SERVO
    servo_init();
    #endif

    #ifdef ENABLE_VEDIO
    if(video_init(0) < 0) {
        printf("VIDEO INIT failed!\n");
        return -1;
    }
    printf("VIDEO INIT!\n");
    #endif

    fd_wtdog = open("/dev/watchdog", O_WRONLY);  
    if(fd_wtdog == -1) {  
        int err = errno;  
        printf("\n!!! FAILED to open /dev/watchdog, errno: %d, %s\n", err, strerror(err));  
        syslog(LOG_WARNING, "FAILED to open /dev/watchdog, errno: %d, %s", err, strerror(err));  
    }
    
    return 0;
}

static safe_exit()
{
    r_timer_safe_exit();
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
    #ifdef ENABLE_SERVO
    pthread_t servo_thread;
    #endif

	#ifdef ENABLE_MOTOR
    pthread_t speed_detect_thd;
	#endif

    #ifdef ENABLE_MPU6050
    pthread_t mpu_thread;
    #endif

    #ifdef ENABLE_WIFI_LOC
    pthread_t wifi_loc_thd;
    #endif

    if(system_init() < 0) {
		printf("system_init failed!\n");
        return -1;
	}

    #ifdef ENABLE_VIDEO
    video_start(0);
    #endif

    #ifdef ENABLE_LED
    set_timer(0, R_TIMER_LOOP, 10, led_shining, NULL);
    #endif

    #ifdef ENABLE_USONIC
    set_timer(R_MSG_USONIC_DETECT, R_TIMER_LOOP, 50, usonic_detect, NULL);
    #endif

//    set_timer(R_MSG_SERVO_0, R_TIMER_LOOP, 1000, servo_0_rotating, 0);
//    set_timer(R_MSG_SERVO_0, R_TIMER_LOOP, 1000, regular_servo_rotating, NULL);

    pthread_create(&tm_thread, NULL, &timer_scan_thread, NULL);
    pthread_create(&commu_thread, NULL, &commu_proc, NULL);

    #ifdef ENABLE_SERVO
    pthread_create(&servo_thread, NULL, &servo_proc, NULL);
    #endif

	#ifdef ENABLE_MOTOR
    pthread_create(&speed_detect_thd, NULL, &detect_speed, NULL);
	#endif

    #ifdef ENABLE_MPU6050
    pthread_create(&mpu_thread, NULL, &mpu6050_detect_with_dmp, NULL);
    #endif

    #ifdef ENABLE_WIFI_LOC
    pthread_create(&wifi_loc_thd, NULL, &wifi_location, NULL);
    #endif
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

	#ifdef ENABLE_MOTOR
    pthread_join(speed_detect_thd, NULL);
	#endif

    pthread_join(tm_thread, NULL);

    #ifdef ENABLE_MPU6050
    pthread_join(mpu_thread, NULL);
    #endif

    pthread_join(commu_thread, NULL);

    #ifdef ENABLE_SERVO
    pthread_join(servo_thread, NULL);
    #endif

    gpio_exit();

    return 0;
}

