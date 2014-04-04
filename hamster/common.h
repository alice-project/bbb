#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char  u_int8;
typedef unsigned short u_int16;
typedef unsigned int   u_int32;
typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;



typedef int (*msg_func)(void *p);

typedef int (*timer_func)(void *p);

#endif

