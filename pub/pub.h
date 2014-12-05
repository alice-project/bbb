#ifndef __PUB_H__
#define __PUB_H__

#define MAX_HAMTERS 16

struct s_com
{
    unsigned char code;
    unsigned char id;
    unsigned int  flags;
};

struct s_request_base
{
    unsigned char name[16];
};

struct s_mc_ipaddr
{
    unsigned int ipaddr;
};

#define BA_LIGHT_ON  1
#define BA_LIGHT_OFF 0
struct s_base_light
{
    unsigned int on_off;
};


/**/
#define LEFT_SIDE  0
#define RIGHT_SIDE 1

#define POSITIVE_DIR  0
#define NEGATIVE_DIR  1

#define START_ACTION  0
#define STOP_ACTION   1
struct s_base_motion
{
    unsigned int side;
    unsigned int action;
    unsigned int dir;
};

struct s_base_pwm_duty
{
    unsigned int cmd;
};

struct s_base_pwm_freq
{
    unsigned int cmd;
};

#define MUL_IPADDR  "224.1.1.1"
#define BASE_PORT   7878

/* fixed message length between BASE and HM */
#define BUFLEN 256

/* Operation Mode */
#define MANUAL_MODE 0
#define AUTO_MODE   1

/*  */

enum {
    /* 0 ~ 127: hm to base */
    HM_REQUEST_BASE = 0,
    HM_REPORTING,

    /* 128 ~ 255: base to hm */
    BA_MC_IPADDR = 128,
    BA_GC_SETTINGS,      /* General Control Settings */
    BA_LIGHT_CMD,
    BA_MOTION_CMD,
    BA_TEST_CMD,
    BA_PWM_DUTY_CMD,
    BA_PWM_FREQ_CMD,
};


#endif

