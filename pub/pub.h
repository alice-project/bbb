#ifndef __PUB_H__
#define __PUB_H__

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

#define MAX_HAMTERS 16

#define MUL_IPADDR  "224.1.1.1"
#define HM_PORT1  30825
#define HM_PORT2  30725

struct s_com
{
    u_int8 code;
    u_int8 id;
    u_int16  flags;
};

struct s_hm_video
{
    u_int32 size;
    u_int8 data[0];
};
#define MAX_VIDEO_FRAME_MSG  (64*1024+sizeof(struct s_com)+sizeof(struct s_hm_video))

struct s_request_base
{
    u_int8 name[16];
};

struct s_hm_distance
{
    u_int32 ssonic_id;
    u_int32 distance;
};

struct s_mc_ipaddr
{
    u_int32 ipaddr;
};

#define BA_LIGHT_ON  1
#define BA_LIGHT_OFF 0
struct s_base_light
{
    u_int32 on_off;
};


struct s_base_camera_cmd
{
    u_int32 on_off;
};


/**/
#define LEFT_SIDE  0
#define RIGHT_SIDE 1

#define POSITIVE_DIR  0
#define NEGATIVE_DIR  1

#define NULL_ACTION       0
#define START_ACTION      1
#define STOP_ACTION       2
#define SET_DUTY_ACTION   3
struct s_base_motion
{
    u_int32 left_action;
    u_int32 left_data;
    u_int32 right_action;
    u_int32 right_data;
};

struct s_base_servo_cmd
{
    u_int32 id;
    u_int32 cmd;  // 正转/反转
    u_int32 angle;  //角度
};


/* fixed message length between BASE and HM */
#define BUFLEN 256

/* Operation Mode */
#define MANUAL_MODE 0
#define AUTO_MODE   1
struct s_hm_mode
{
    u_int32 mode;
};
/*  */

enum {
    /* 0 ~ 31: hm to base */
    HM_REQUEST_BASE = 0,
    HM_REPORTING,
    HM_CAMERA,
    HM_DISTANCE,

    /* 32 ~ 63: base to hm */
    BA_MC_IPADDR = 32,
    BA_GC_SETTINGS,      /* General Control Settings */
    BA_LIGHT_CMD,
    BA_MOTION_CMD,
    BA_SERVO_CMD,
    BA_TEST_CMD,
    BA_PWM_DUTY_CMD,
    BA_PWM_FREQ_CMD,
    BA_CAMERA_CMD,
    BA_REPLY_HM_ADDR,

    MC_REQUEST_HM_ADDR,
    BA_RAWTEXT_CMD,
};

struct s_base_info
{
    int fd;
    struct sockaddr_in m_addr;
};



#endif

