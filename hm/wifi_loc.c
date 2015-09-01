#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <iwlib.h>

#include "wifi_loc.h"

struct wap_loc AP_available[40];

int match_ap(char apid[])
{
    int i;
    for(i=0;i<sizeof(AP_available)/sizeof(struct wap_loc);i++) {
        if(AP_available[i].flag==0)
            return i;
        if(memcmp(apid, AP_available[i].addr, sizeof(apid))==0)
            return i;
    }
    return -1;
}

void keep_ap(struct wap_loc *dst, wireless_scan *src, char addr[])
{
    int m;
    double avg=0.0;
    dst->flag=1;
    memcpy(dst->essid, src->b.essid, sizeof(dst->essid));
    memcpy(dst->addr, addr, sizeof(dst->addr));
    for(m=0;m<MAX_SIGNAL_SAVED;m++) {
        if(dst->signal[m]==0) {
            dst->signal[m]=src->stats.qual.level;
            avg = (dst->signal_avg*m+src->stats.qual.level)/(m+1);
            dst->signal_avg = avg;
            return;
        }
    }
    for(m=1;m<MAX_SIGNAL_SAVED;m++) {
        dst->signal[m-1]=dst->signal[m-1];
    }
    avg = 0.0;
    for(m=0;m<MAX_SIGNAL_SAVED;m++) {
        avg += dst->signal[m];
        
    }
    dst->signal_avg = avg / MAX_SIGNAL_SAVED;
}

int location(int sock, iwrange *range)
{
    wireless_scan_head head;
    wireless_scan *ap;
    char apid[13];
    int idx;

    /* Perform the scan */
    if (iw_scan(sock, "wlan0", range->we_version_compiled, &head) < 0) {
        printf("Error during iw_scan. Aborting.\n");
    }

    /* Traverse the results */
    ap = head.result;
    while (NULL != ap) {
        sprintf(apid, "%02x%02x%02x%02x%02x%02x", ap->ap_addr.sa_data[0]&0xff, ap->ap_addr.sa_data[1]&0xff, 
                                                  ap->ap_addr.sa_data[2]&0xff, ap->ap_addr.sa_data[3]&0xff, 
                                                  ap->ap_addr.sa_data[4]&0xff, ap->ap_addr.sa_data[5]&0xff);
        idx=match_ap(apid);
        if(idx>=0) {
            keep_ap(&AP_available[idx], ap, apid);
        }
        ap = ap->next;
    }

    return 0;
}

void init_ap()
{
    memset(AP_available, 0, sizeof(AP_available));
}

void print_single_ap(struct wap_loc *ap)
{
    printf("AP: %s, ESSID: %s, avg of signal=%d, last signal=%d\n", ap->addr, ap->essid, ap->signal_avg, ap->signal[0]);
    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", ap->signal[0], ap->signal[1], ap->signal[2], ap->signal[3], ap->signal[4],
                      ap->signal[5], ap->signal[6], ap->signal[7], ap->signal[8], ap->signal[9]);
    printf("\n");
}

void print_all_ap()
{
    int i;
    for(i=0;i<sizeof(AP_available)/sizeof(struct wap_loc);i++) {
        if(AP_available[i].flag==1)
            print_single_ap(&AP_available[i]);
    }
}

void *wifi_location(void *data)
{
    iwrange range;
    int sock;

    init_ap();

    /* Open socket to kernel */
    sock = iw_sockets_open();

    /* Get some metadata to use for scanning */
    if (iw_get_range_info(sock, "wlan0", &range) < 0) {
        printf("Error during iw_get_range_info. Aborting.\n");
        return NULL;
    }
    while(1)
    {
        if(location(sock, &range)<0)
            continue;
       sleep(1);
    }
    iw_sockets_close(sock);

    print_all_ap();
    return NULL;
}


