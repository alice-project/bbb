#include <stdio.h> 
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "i2c_func.h"


int i2c_open(unsigned char bus, unsigned char addr)
{
    int file;
    char filename[16];
    sprintf(filename,"/dev/i2c-%d", bus);
    if ((file = open(filename,O_RDWR)) < 0)
    {
        fprintf(stderr, "i2c_open open error(%d): %s\n", errno, strerror(errno));
        return(file);
    }
//printf("========\n");
//printf("FILE=%s,__LINE__=%d,addr=0x%x\n", __FILE__, __LINE__,addr);

    if (ioctl(file,I2C_SLAVE,addr) < 0)
    {
        fprintf(stderr, "i2c_open ioctl error(%d): %s\n",errno, strerror(errno));
        return(-1);
    }
//printf("FILE=%s,__LINE__=%d,addr=0x%x\n", __FILE__, __LINE__,addr);

    return(file);
}

int i2c_write(int handle, unsigned char* buf, unsigned int length)
{
    if (write(handle, buf, length) != length)
    {
        fprintf(stderr, "i2c_write_bytes error(%d): %s\n",errno, strerror(errno));
        return(-1);
    }
    return(length);
}

int i2c_read(int handle, unsigned char* buf, unsigned int length)
{
    if (read(handle, buf, length) != length)
    {
        fprintf(stderr, "i2c_read error(%d): %s\n",errno, strerror(errno));
        return(-1);
    }
    return(length);
}


int i2c_close(int handle)
{
    if ((close(handle)) != 0)
    {
        fprintf(stderr, "i2c_close error(%d): %s\n",errno, strerror(errno));
        return(-1);
    }
  return(0);
}

int i2c_write_read(int handle,
                   unsigned char addr_w, unsigned char *buf_w, unsigned int len_w,
                   unsigned char addr_r, unsigned char *buf_r, unsigned int len_r)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[2];
	
	msgs[0].addr=addr_w;
	msgs[0].len=len_w;
	msgs[0].flags=0;
	msgs[0].buf=buf_w;
	
	msgs[1].addr=addr_r;
	msgs[1].len=len_r;
	msgs[1].flags=1;
	msgs[1].buf=buf_r;
	
	msgset.nmsgs=2;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
    {
        fprintf(stderr, "i2c_write_read error(%d): %s\n",errno,strerror(errno));
        return -1;
    } 
    return(len_r);
}

int i2c_write_ignore_nack(int handle,
                          unsigned char addr_w, unsigned char* buf, unsigned int length)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[1];
	
	msgs[0].addr=addr_w;
	msgs[0].len=length;
	msgs[0].flags=0 | I2C_M_IGNORE_NAK;
	msgs[0].buf=buf;
	
	msgset.nmsgs=1;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
    {
        fprintf(stderr, "i2c_write_ignore_nack error(%d): %s\n",errno,strerror(errno));
        return -1;
    }
    return(length);
}

int i2c_read_no_ack(int handle, 
                    unsigned char addr_r, unsigned char* buf, unsigned int length)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[1];
	
	msgs[0].addr=addr_r;
	msgs[0].len=length;
	msgs[0].flags=I2C_M_RD | I2C_M_NO_RD_ACK;
	msgs[0].buf=buf;
	
	msgset.nmsgs=1;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
    {
        fprintf(stderr, "i2c_read_no_ack error(%d): %s\n",errno,strerror(errno));
        return -1;
    } 
    return(length);
}

int delay_ms(unsigned int msec)
{
    int ret;
    struct timespec a;
    if (msec>999)
    {
        fprintf(stderr, "delay_ms error: delay value needs to be less than 999\n");
        msec=999;
    }
    a.tv_nsec=((long)(msec))*1E6d;
    a.tv_sec=0;
    if ((ret = nanosleep(&a, NULL)) != 0)
    {
        fprintf(stderr, "delay_ms error(%d): %s\n",errno , strerror(errno));
    }
    return(0);
}


static int i2c_smbus_access(int file, char read_write, unsigned char command, 
                                     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}


int i2c_smbus_write_quick(int file, unsigned char value)
{
	return i2c_smbus_access(file,value,0,I2C_SMBUS_QUICK,NULL);
}
	
int i2c_smbus_read_byte(int file)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
		return -1;
	else
		return 0x0FF & data.byte;
}

int i2c_smbus_write_byte(int file, unsigned char value)
{
	return i2c_smbus_access(file,I2C_SMBUS_WRITE,value,
	                        I2C_SMBUS_BYTE,NULL);
}

int i2c_smbus_read_byte_data(int file, unsigned char command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     I2C_SMBUS_BYTE_DATA,&data))
		return -1;
	else
		return 0x0FF & data.byte;
}

int i2c_smbus_write_byte_data(int file, unsigned char command, unsigned char value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                        I2C_SMBUS_BYTE_DATA, &data);
}

int i2c_smbus_read_word_data(int file, unsigned char command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     I2C_SMBUS_WORD_DATA,&data))
		return -1;
	else
		return 0x0FFFF & data.word;
}

int i2c_smbus_write_word_data(int file, unsigned char command, 
                                              unsigned short value)
{
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                        I2C_SMBUS_WORD_DATA, &data);
}

int i2c_smbus_process_call(int file, unsigned char command, unsigned short value)
{
	union i2c_smbus_data data;
	data.word = value;
	if (i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                     I2C_SMBUS_PROC_CALL,&data))
		return -1;
	else
		return 0x0FFFF & data.word;
}


/* Returns the number of read bytes */
int i2c_smbus_read_block_data(int file, unsigned char command, 
                              unsigned char *values)
{
	union i2c_smbus_data data;
	int i;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     I2C_SMBUS_BLOCK_DATA,&data))
		return -1;
	else {
		for (i = 1; i <= data.block[0]; i++)
        {
			values[i-1] = data.block[i];
        }
		return data.block[0];
	}
}

int i2c_smbus_write_block_data(int file, unsigned char command, unsigned char length, const unsigned char *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > 32)
		length = 32;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                        I2C_SMBUS_BLOCK_DATA, &data);
}

/* Returns the number of read bytes */
/* Until kernel 2.6.22, the length is hardcoded to 32 bytes. If you
   ask for less than 32 bytes, your code will only work with kernels
   2.6.23 and later. */
int i2c_smbus_read_i2c_block_data(int file, unsigned char command,
                                  unsigned char length, unsigned char *values)
{
	union i2c_smbus_data data;
	int i;

	if (length > 32)
		length = 32;
	data.block[0] = length;

	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     length == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN :
	                      I2C_SMBUS_I2C_BLOCK_DATA,&data))
		return -1;
	else {
		for (i = 1; i <= data.block[0]; i++)
        {
			values[i-1] = data.block[i];
        }
		return data.block[0];
	}
}

int i2c_smbus_write_i2c_block_data(int file, unsigned char command,
                                                   unsigned char length,
                                                   const unsigned char *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > 32)
		length = 32;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                        I2C_SMBUS_I2C_BLOCK_BROKEN, &data);
}

/* Returns the number of read bytes */
int i2c_smbus_block_process_call(int file, unsigned char command,
                                                 unsigned char length, unsigned char *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > 32)
		length = 32;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	if (i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
	                     I2C_SMBUS_BLOCK_PROC_CALL,&data))
		return -1;
	else {
		for (i = 1; i <= data.block[0]; i++)
			values[i-1] = data.block[i];
		return data.block[0];
	}
}


