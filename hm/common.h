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


typedef int (*msg_func)(void *p);

typedef int (*timer_func)(void *p);

struct s_base_info
{
    int fd;
    struct sockaddr_in m_addr;
};

#endif

