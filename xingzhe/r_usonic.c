#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "pub.h"
#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "gpio.h"
#include "r_usonic.h"

#include "i2c_func.h"

#define USONIC_0_ADDR  (0x70)

#define MAX_USONIC_DETECT_TM  0x1ffff0

unsigned char usonic_addr[MAX_USONIC];
u_int32 usonic_distance[MAX_USONIC];

extern pthread_mutex_t mutex_i2c;
int change_usonic_addr(int fd, unsigned char new_addr)
{
    unsigned char buf[3];
    if(fd < 0)
        return -1;
    
    if(i2c_smbus_write_byte_data(fd, 2, 0x9a)<0)
        printf("WRITE error(%d):%s\n", errno, strerror(errno));
    delay_ms(1);
    if(i2c_smbus_write_byte_data(fd, 2, 0x92)<0)
        printf("WRITE error(%d):%s\n", errno, strerror(errno));
    delay_ms(1);
    if(i2c_smbus_write_byte_data(fd, 2, 0x9e)<0)
        printf("WRITE error(%d):%s\n", errno, strerror(errno));
    delay_ms(1);
    if(i2c_smbus_write_byte_data(fd, 2, new_addr)<0)
        printf("WRITE error(%d):%s\n", errno, strerror(errno));

    return 0;
}

int usonic_init()
{
    usonic_addr[LEFT_FRONT] = USONIC_0_ADDR;     
    return 0;
}

int usonic_regist()
{
	return 0;
}

extern int report_distance_info(struct s_sxz_distance *data);
int usonic_detect_item(int item)
{
    int dist_hi, dist_lo;
    struct s_sxz_distance data;

	pthread_mutex_lock(&mutex_i2c);

    int fd_usonic = i2c_open(1, usonic_addr[item]);

    if(i2c_smbus_write_byte_data(fd_usonic, 2, 0xb4) < 0)
       printf("WRITE error(%d):%s\n", errno, strerror(errno));
    delay_ms(80);

    dist_hi = i2c_smbus_read_byte_data(fd_usonic, 2);
    if(dist_hi < 0)
        dist_hi = 0;

    dist_lo = i2c_smbus_read_byte_data(fd_usonic, 3);
    if(dist_lo < 0)
        dist_lo = 0;

    if(fd_usonic >= 0)
        i2c_close(fd_usonic);

	pthread_mutex_unlock(&mutex_i2c);

    usonic_distance[item] = dist_hi*255 + dist_lo;

    data.ssonic_id=0;
    data.distance=usonic_distance[item];
    report_distance_info(&data);

printf("USONIC: distance=%d, hi=%d,lo=%d\n", usonic_distance[0], dist_hi, dist_lo);
}

int usonic_detect(void *data)
{
    usonic_detect_item(LEFT_FRONT);
    return 0;
}


