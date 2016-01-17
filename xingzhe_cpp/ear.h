#ifndef __QIANLIYAN_H__
#define __QIANLIYAN_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "pub.h"

#include "gpio.h"
#include "r_usonic.h"

#include "i2c_func.h"

#define USONIC_0_ADDR  (0x70)

#define MAX_USONIC_DETECT_TM  0x1ffff0

class XZ_Ear {
public:
    XZ_Ear(u_int8 addr);
    ~XZ_Ear();

    void hear();
    
    int change_usonic_addr(unsigned char new_addr);
    int usonic_detect_item();

private:
    u_int8 usonic_addr;
    u_int32 usonic_distance;
    pthread_mutex_t mutex_i2c;

    int fd;

protected:



};


#endif

