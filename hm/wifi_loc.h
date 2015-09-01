#ifndef __WIFI_LOC_H__
#define __WIFI_LOC_H__

#include <iwlib.h>
#include "common.h"

#define MAX_SIGNAL_SAVED  10

struct wap_loc {
    int flag;
    char essid[IW_ESSID_MAX_SIZE + 2];
    char addr[13];
    int8 signal[MAX_SIGNAL_SAVED];
    int8 signal_avg;
};

int location();

#endif

