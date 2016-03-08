#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <netdb.h>

typedef unsigned char  u_int8;
typedef unsigned short u_int16;
typedef unsigned int   u_int32;
typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;

typedef int (*MSG_HANDLER)(void *msg);
typedef int (*msg_func)(void *p);

typedef int (*timer_func)(void *p);

struct s_xz_state
{
    unsigned int m_moving;
    unsigned int m_control;
    unsigned int m_camera;
    unsigned int m_usonic0;
    unsigned int m_usonic1;
    unsigned int m_usonic2;
    unsigned int m_usonic3;
    unsigned int m_left_pwm;
    unsigned int m_right_pwm;
};

struct s_xz_state_flag
{
    unsigned int moving_flag :1 ;
    unsigned int control_flag:1;
    unsigned int camera_flag :1;
    unsigned int rsv         :29;
};

#define MY_NAME  "Hanma"

#define min(a,b) ((a)<(b)?(a):(b))

int regist_cmd_cb(u_int8 id, MSG_HANDLER cb);

#endif

