#ifndef __R_USONIC_H__
#define __R_USONIC_H__

#include "common.h"

enum {
    LEFT_FRONT = 0,
    RIGHT_FRONT,
    LEFT_BACK,
    RIGHT_BACK,
    
    MAX_USONIC,
};

enum {
    USONIC_IDLE = 0,
    USONIC_WAIT,
    USONIC_BUSY,
    USONIC_FIN,
};

int usonic_sensor_scan(void *data);
int usonic_regist();

#endif

