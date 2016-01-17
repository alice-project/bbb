#include "ear.h"

XZ_Ear::XZ_Ear(u_int8 addr)
{
    usonic_addr = addr;
}

XZ_Ear::~XZ_Ear()
{
}

int XZ_Ear::change_usonic_addr(unsigned char new_addr)
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

int XZ_Ear::usonic_detect_item()
{
    int dist_hi, dist_lo;
    struct s_sxz_distance data;

	pthread_mutex_lock(&mutex_i2c);

    int fd_usonic = i2c_open(1, usonic_addr);

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

    usonic_distance = dist_hi*255 + dist_lo;

    data.ssonic_id=0;
    data.distance=usonic_distance;
//    report_distance_info(&data);

printf("USONIC: distance=%d, hi=%d,lo=%d\n", usonic_distance, dist_hi, dist_lo);
}

