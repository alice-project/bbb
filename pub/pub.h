#ifndef __PUB_H__
#define __PUB_H__

struct s_com
{
    unsigned char ver;
    unsigned char code;
    unsigned char id;
    unsigned char len;
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
    /* 0 ~ 127: hamster to base */
    HA_REQUEST_BASE = 0,

    /* 128 ~ 255: base to hamster */
    BA_MC_IPADDR = 128,
};

#endif

