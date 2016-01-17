#ifndef __XINGZHE_H__
#define __XINGZHE_H__

#include "eye.h"
#include "ear.h"
#include "foot.h"

struct commu_param {
    struct s_base_info m_base;
    u_int32 d_base_ipaddr;
};

class xingzhe {
public:
    xingzhe();
    xingzhe(char *name);
    ~xingzhe();

private:
    void init();
    void get_my_addr();
    int wait_for_base();

private:
    XZ_Eye *eye = NULL;  // camera
    XZ_Ear *ear = NULL;  // usonic sensor
    XZ_Foot *left_foot = NULL;  //wheel
    XZ_Foot *right_foot = NULL;  //wheel

    pthread_t commu_thd;

    char *s_my_ipaddr;
    u_int32 d_my_ipaddr;
    u_int32 d_base_ipaddr;
    
//    head(); // servo;light
//    brain(); // decision
//    speed(); // speed
//    westbus(); // communication with base
    

protected:

public:
    
};


#endif

