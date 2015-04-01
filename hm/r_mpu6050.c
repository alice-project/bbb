#include <stdio.h>
#include <stdlib.h>
#include "i2c_func.h"
#include "r_mpu6050.h"

int fd_mpu6050 = -1;
unsigned int accel_x, accel_y, accel_z;
unsigned int gyro_x, gyro_y, gyro_z;

/** Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources can
 * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillator
 * or an external source is chosen as the clock source, the MPU-60X0 can operate
 * in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of
 * the gyroscopes (or an external clock source) as the clock reference for
 * improved stability. The clock source can be selected according to the following table:
 *
 * <pre>
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * </pre>
 *
 * @param source New clock source setting
 * @see get_clock_source()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CLKSEL_BIT
 * @see MPU6050_PWR1_CLKSEL_LENGTH
 */
static void set_clock_source(unsigned char source) {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_PWR_MGMT_1);
    value = (value & 0x5) | (source & 0x5);
    i2c_smbus_write_byte_data(fd_mpu6050, MPU6050_RA_PWR_MGMT_1, value);
}

/** Get clock source setting.
 * @return Current clock source setting
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CLKSEL_BIT
 * @see MPU6050_PWR1_CLKSEL_LENGTH
 */
static unsigned char get_clock_source() {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_PWR_MGMT_1);

    return (value & 0x5);
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see get_full_scale_gyro_range()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
static void set_full_scale_gyro_range(unsigned char range) {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_GYRO_CONFIG);
    value = (value & 0x18) | (range & 0x18);
    i2c_smbus_write_byte_data(fd_mpu6050, MPU6050_RA_GYRO_CONFIG, value);
}

/** Get full-scale gyroscope range.
 * The FS_SEL parameter allows setting the full-scale range of the gyro sensors,
 * as described in the table below.
 *
 * <pre>
 * 0 = +/- 250 degrees/sec
 * 1 = +/- 500 degrees/sec
 * 2 = +/- 1000 degrees/sec
 * 3 = +/- 2000 degrees/sec
 * </pre>
 *
 * @return Current full-scale gyroscope range setting
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
static unsigned char get_full_scale_gyro_range() {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_GYRO_CONFIG);
    return (value>>3)&0x3;
}


/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see get_full_scale_accel_range()
 */
static void set_full_scale_accel_range(unsigned char range) {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_ACCEL_CONFIG);
    value = (value & 0x18) | (range & 0x18);
    i2c_smbus_write_byte_data(fd_mpu6050, MPU6050_RA_GYRO_CONFIG, value);
}

/** Get full-scale accelerometer range.
 * The FS_SEL parameter allows setting the full-scale range of the accelerometer
 * sensors, as described in the table below.
 *
 * <pre>
 * 0 = +/- 2g
 * 1 = +/- 4g
 * 2 = +/- 8g
 * 3 = +/- 16g
 * </pre>
 *
 * @return Current full-scale accelerometer range setting
 * @see MPU6050_ACCEL_FS_2
 * @see MPU6050_RA_ACCEL_CONFIG
 * @see MPU6050_ACONFIG_AFS_SEL_BIT
 * @see MPU6050_ACONFIG_AFS_SEL_LENGTH
 */
static unsigned char get_full_scale_accel_range() {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_ACCEL_CONFIG);
    return (value>>3)&0x3;
}


/** Set sleep mode status.
 * @param enabled New sleep mode enabled status
 * @see get_sleep_enabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
static void set_sleep_enabled(unsigned char enabled) {
    int value = i2c_smbus_read_byte_data(fd_mpu6050, MPU6050_RA_PWR_MGMT_1);
    value = (value & 0x40) | (enabled & 0x40);
    i2c_smbus_write_byte_data(fd_mpu6050, MPU6050_RA_PWR_MGMT_1, value);
}


void mpu6050_init() 
{
    fd_mpu6050 = i2c_open(2, MPU6050_DEFAULT_ADDRESS);

    set_clock_source(MPU6050_CLOCK_PLL_XGYRO);
    set_full_scale_gyro_range(MPU6050_GYRO_FS_250);
    set_full_scale_accel_range(MPU6050_ACCEL_FS_2);
    set_sleep_enabled(0); // thanks to Jack Elston for pointing this one out!

    if(fd_mpu6050 >= 0)
        i2c_close(fd_mpu6050);
}

static unsigned int get_measurement(unsigned char r_hi, unsigned char r_lo)
{
    int hi, lo;
    hi = i2c_smbus_read_byte_data(fd_mpu6050, r_hi);
    hi = (hi > 0)?hi:0;
    lo = i2c_smbus_read_byte_data(fd_mpu6050, r_lo);
    lo = (lo > 0)?lo:0;

    return ((hi<<8)+lo);
}

int mpu6050_detect(void *data)
{
    fd_mpu6050 = i2c_open(2, MPU6050_DEFAULT_ADDRESS);

    accel_x = get_measurement(0x3b, 0x3c);
    accel_y = get_measurement(0x3d, 0x3e);
    accel_z = get_measurement(0x3f, 0x40);
    gyro_x = get_measurement(0x43, 0x44);
    gyro_y = get_measurement(0x45, 0x46);
    gyro_z = get_measurement(0x47, 0x48);

printf("accel: %d-%d-%d\n", accel_x, accel_y, accel_z);
printf("gyro: %d-%d-%d\n", gyro_x, gyro_y, gyro_z);

    if(fd_mpu6050 >= 0)
        i2c_close(fd_mpu6050);

    return 0;
}


