#ifndef __PUB_H__
#define __PUB_H__

#define MAX_HAMTERS 16

struct s_com
{
    unsigned char ver;
    unsigned char code;
    unsigned char id;
    unsigned char len;
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

#define MUL_IPADDR  "224.1.1.1"
#define BASE_PORT   7878
#define BUFLEN 256

enum {
    /* 0 ~ 127: hm to base */
    HM_REQUEST_BASE = 0,
    HM_REPORTING,

    /* 128 ~ 255: base to hm */
    BA_MC_IPADDR = 128,
    BA_INSTRUCT_MOVE,
};


#endif

