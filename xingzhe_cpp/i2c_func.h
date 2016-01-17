#include <linux/types.h>
#include <sys/ioctl.h>

#ifndef __I2C_FUNC_H__
#define __I2C_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

    int i2c_open(unsigned char bus, unsigned char addr);
    int i2c_write(int handle, unsigned char* buf, unsigned int length);
    int i2c_read(int handle, unsigned char* buf, unsigned int length);
    int i2c_write_read(int handle,
                       unsigned char addr_w, unsigned char *buf_w, unsigned int len_w,
                       unsigned char addr_r, unsigned char *buf_r, unsigned int len_r);
    int i2c_write_ignore_nack(int handle,
                              unsigned char addr_w, unsigned char* buf, unsigned int length);
    int i2c_read_no_ack(int handle, 
                        unsigned char addr_r, unsigned char* buf, unsigned int length);
    int i2c_write_byte(int handle, unsigned char val);
    int i2c_read_byte(int handle, unsigned char* val);

    int i2c_close(int handle);

    int i2c_smbus_write_byte(int file, unsigned char value);
    int i2c_smbus_read_byte_data(int file, unsigned char command);
    int i2c_smbus_read_byte(int file);
    int i2c_smbus_write_byte_data(int file, unsigned char command, unsigned char value);
    int delay_ms(unsigned int msec);

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_FUNC_H__ */

