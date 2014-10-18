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

#define MUL_IPADDR  "224.1.1.1"
#define BASE_PORT   7878

/* fixed message length between BASE and HM */
#define BUFLEN 256

#define MANUAL_MODE 0
#define AUTO_MODE   1

enum {
    /* 0 ~ 127: hm to base */
    HM_REQUEST_BASE = 0,
    HM_REPORTING,

    /* 128 ~ 255: base to hm */
    BA_MC_IPADDR = 128,
    BA_GC_SETTINGS,      /* General Control Settings */
    BA_LIGHT_CMD,
    BA_INSTRUCTION_MOVE_FORWARD,
    BA_INSTRUCTION_MOVE_BACKWARD,
    BA_INSTRUCTION_STOP,
};


#endif

