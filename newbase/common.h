#ifndef __COMMON_H__
#define __COMMON_H__

struct s_hm
{
    int fd;
    unsigned int id;
    char name[16];
    struct sockaddr_in c_addr;
};

#endif

