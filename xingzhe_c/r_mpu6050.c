#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <poll.h>

#include "common.h"
#include "gpio.h"
#include "i2c_func.h"
#include "r_mpu6050.h"
#include "mpu_dmp_motion_drv.h"

int fd_mpu6050 = -1;
int accel_x, accel_y, accel_z;
int gyro_x, gyro_y, gyro_z;
int temperature=0;
extern pthread_mutex_t mutex_i2c;

const int gyro_pin[][2] = {
    /* connector, PIN */
    {8, 7},
};

#define MPU6050
#define MPU_DEBUG

#define DEFAULT_MPU_HZ  (100)	// starting sampling rate

#ifdef MPU9150
#ifndef MPU6050
#define MPU6050
#endif                          /* #ifndef MPU6050 */
#ifdef AK8963_SECONDARY
#error "MPU9150 and AK8963_SECONDARY cannot both be defined."
#elif !defined AK8975_SECONDARY /* #ifdef AK8963_SECONDARY */
#define AK8975_SECONDARY
#endif                          /* #ifdef AK8963_SECONDARY */
#elif defined MPU9250           /* #ifdef MPU9150 */
#ifndef MPU6500
#define MPU6500
#endif                          /* #ifndef MPU6500 */
#ifdef AK8975_SECONDARY
#error "MPU9250 and AK8975_SECONDARY cannot both be defined."
#elif !defined AK8963_SECONDARY /* #ifdef AK8975_SECONDARY */
#define AK8963_SECONDARY
#endif                          /* #ifdef AK8975_SECONDARY */
#endif                          /* #ifdef MPU9150 */

#if (defined AK8975_SECONDARY) || (defined AK8963_SECONDARY)
#define AK89xx_SECONDARY
#else
#endif

/* Hardware registers needed by driver. */
struct gyro_reg_s
{
	u_int8 who_am_i;
	u_int8 rate_div;
	u_int8 lpf;
	u_int8 prod_id;
	u_int8 user_ctrl;
	u_int8 fifo_en;
	u_int8 gyro_cfg;
	u_int8 accel_cfg;
	u_int8 accel_cfg2;
	u_int8 lp_accel_odr;
	u_int8 motion_thr;
	u_int8 motion_dur;
	u_int8 fifo_count_h;
	u_int8 fifo_r_w;
	u_int8 raw_gyro;
	u_int8 raw_accel;
	u_int8 temp;
	u_int8 int_enable;
	u_int8 dmp_int_status;
	u_int8 int_status;
	u_int8 accel_intel;
	u_int8 pwr_mgmt_1;
	u_int8 pwr_mgmt_2;
	u_int8 int_pin_cfg;
	u_int8 mem_r_w;
	u_int8 accel_offs;
	u_int8 i2c_mst;
	u_int8 bank_sel;
	u_int8 mem_start_addr;
	u_int8 prgm_start_h;
#ifdef AK89xx_SECONDARY
	u_int8 s0_addr;
	u_int8 s0_reg;
	u_int8 s0_ctrl;
	u_int8 s1_addr;
	u_int8 s1_reg;
	u_int8 s1_ctrl;
	u_int8 s4_ctrl;
	u_int8 s0_do;
	u_int8 s1_do;
	u_int8 i2c_delay_ctrl;
	u_int8 raw_compass;
	/* The I2C_MST_VDDIO bit is in this register. */
	u_int8 yg_offs_tc;
#endif
};

/* Information specific to a particular device. */
struct hw_s
{
	u_int8 addr;
	u_int16 max_fifo;
	u_int8 num_reg;
	u_int16 temp_sens;
	int16_t temp_offset;
	u_int16 bank_size;
#ifdef AK89xx_SECONDARY
	u_int16 compass_fsr;
#endif
};

/* When entering motion interrupt mode, the driver keeps track of the
 * previous state so that it can be restored at a later time.
 * TODO: This is tacky. Fix it.
 */
struct motion_int_cache_s
{
	u_int16 gyro_fsr;
	u_int8 accel_fsr;
	u_int16 lpf;
	u_int16 sample_rate;
	u_int8 sensors_on;
	u_int8 fifo_sensors;
	u_int8 dmp_on;
};

/* Cached chip configuration data.
 * TODO: A lot of these can be handled with a bitmask.
 */
struct chip_cfg_s
{
	/* Matches gyro_cfg >> 3 & 0x03 */
	u_int8 gyro_fsr;
	/* Matches accel_cfg >> 3 & 0x03 */
	u_int8 accel_fsr;
	/* Enabled sensors. Uses same masks as fifo_en, NOT pwr_mgmt_2. */
	u_int8 sensors;
	/* Matches config register. */
	u_int8 lpf;
	u_int8 clk_src;
	/* Sample rate, NOT rate divider. */
	u_int16 sample_rate;
	/* Matches fifo_en register. */
	u_int8 fifo_enable;
	/* Matches int enable register. */
	u_int8 int_enable;
	/* 1 if devices on auxiliary I2C bus appear on the primary. */
	u_int8 bypass_mode;
	/* 1 if half-sensitivity.
	 * NOTE: This doesn't beint32_t here, but everything else in hw_s is const,
	 * and this allows us to save some precious RAM.
	 */
	u_int8 accel_half;
	/* 1 if device in low-power accel-only mode. */
	u_int8 lp_accel_mode;
	/* 1 if interrupts are only triggered on motion events. */
	u_int8 int_motion_only;
	struct motion_int_cache_s cache;
	/* 1 for active low interrupts. */
	u_int8 active_low_int;
	/* 1 for latched interrupts. */
	u_int8 latched_int;
	/* 1 if DMP is enabled. */
	u_int8 dmp_on;
	/* Ensures that DMP will only be loaded once. */
	u_int8 dmp_loaded;
	/* Sampling rate used when DMP is enabled. */
	u_int16 dmp_sample_rate;
#ifdef AK89xx_SECONDARY
	/* Compass sample rate. */
	u_int16 compass_sample_rate;
	u_int8 compass_addr;
	int16_t mag_sens_adj[3];
#endif
};

/* Information for self-test. */
struct test_s
{
	u_int32 gyro_sens;
	u_int32 accel_sens;
	u_int8 reg_rate_div;
	u_int8 reg_lpf;
	u_int8 reg_gyro_fsr;
	u_int8 reg_accel_fsr;
	u_int16 wait_ms;
	u_int8 packet_thresh;
	float min_dps;
	float max_dps;
	float max_gyro_var;
	float min_g;
	float max_g;
	float max_accel_var;
};

/* Gyro driver state variables. */
struct gyro_state_s
{
	const struct gyro_reg_s *reg;
	const struct hw_s *hw;
	struct chip_cfg_s chip_cfg;
	const struct test_s *test;
};

/* Filter configurations. */
enum lpf_e
{
	INV_FILTER_256HZ_NOLPF2 = 0,
	INV_FILTER_188HZ,
	INV_FILTER_98HZ,
	INV_FILTER_42HZ,
	INV_FILTER_20HZ,
	INV_FILTER_10HZ,
	INV_FILTER_5HZ,
	INV_FILTER_2100HZ_NOLPF,
	NUM_FILTER
};

/* Full scale ranges. */
enum gyro_fsr_e
{
	INV_FSR_250DPS = 0,
	INV_FSR_500DPS,
	INV_FSR_1000DPS,
	INV_FSR_2000DPS,
	NUM_GYRO_FSR
};

/* Full scale ranges. */
enum accel_fsr_e
{
	INV_FSR_2G = 0,
	INV_FSR_4G,
	INV_FSR_8G,
	INV_FSR_16G,
	NUM_ACCEL_FSR
};

/* Clock sources. */
enum clock_sel_e
{
	INV_CLK_INTERNAL = 0,
	INV_CLK_PLL,
	NUM_CLK
};

/* Low-power accel wakeup rates. */
enum lp_accel_rate_e
{
#ifdef MPU6050
	INV_LPA_1_25HZ,
	INV_LPA_5HZ,
	INV_LPA_20HZ,
	INV_LPA_40HZ
#elif defined MPU6500
	INV_LPA_0_3125HZ,
	INV_LPA_0_625HZ,
	INV_LPA_1_25HZ,
	INV_LPA_2_5HZ,
	INV_LPA_5HZ,
	INV_LPA_10HZ,
	INV_LPA_20HZ,
	INV_LPA_40HZ,
	INV_LPA_80HZ,
	INV_LPA_160HZ,
	INV_LPA_320HZ,
	INV_LPA_640HZ
#endif
};

#define BIT_I2C_MST_VDDIO   (0x80)
#define BIT_FIFO_EN         (0x40)
#define BIT_DMP_EN          (0x80)
#define BIT_FIFO_RST        (0x04)
#define BIT_DMP_RST         (0x08)
#define BIT_FIFO_OVERFLOW   (0x10)
#define BIT_DATA_RDY_EN     (0x01)
#define BIT_DMP_INT_EN      (0x02)
#define BIT_MOT_INT_EN      (0x40)
#define BITS_FSR            (0x18)
#define BITS_LPF            (0x07)
#define BITS_HPF            (0x07)
#define BITS_CLK            (0x07)
#define BIT_FIFO_SIZE_1024  (0x40)
#define BIT_FIFO_SIZE_2048  (0x80)
#define BIT_FIFO_SIZE_4096  (0xC0)
#define BIT_RESET           (0x80)
#define BIT_SLEEP           (0x40)
#define BIT_S0_DELAY_EN     (0x01)
#define BIT_S2_DELAY_EN     (0x04)
#define BITS_SLAVE_LENGTH   (0x0F)
#define BIT_SLAVE_BYTE_SW   (0x40)
#define BIT_SLAVE_GROUP     (0x10)
#define BIT_SLAVE_EN        (0x80)
#define BIT_i2c_smbus_read_byte_data        (0x80)
#define BITS_I2C_MASTER_DLY (0x1F)
#define BIT_AUX_IF_EN       (0x20)
#define BIT_ACTL            (0x80)
#define BIT_LATCH_EN        (0x20)
#define BIT_ANY_RD_CLR      (0x10)
#define BIT_BYPASS_EN       (0x02)
#define BITS_WOM_EN         (0xC0)
#define BIT_LPA_CYCLE       (0x20)
#define BIT_STBY_XA         (0x20)
#define BIT_STBY_YA         (0x10)
#define BIT_STBY_ZA         (0x08)
#define BIT_STBY_XG         (0x04)
#define BIT_STBY_YG         (0x02)
#define BIT_STBY_ZG         (0x01)
#define BIT_STBY_XYZA       (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA)
#define BIT_STBY_XYZG       (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)

#ifdef AK8975_SECONDARY
#define SUPPORTS_AK89xx_HIGH_SENS   (0x00)
#define AK89xx_FSR                  (9830)
#elif defined AK8963_SECONDARY
#define SUPPORTS_AK89xx_HIGH_SENS   (0x10)
#define AK89xx_FSR                  (4915)
#endif

#ifdef AK89xx_SECONDARY
#define AKM_REG_WHOAMI      (0x00)

#define AKM_REG_ST1         (0x02)
#define AKM_REG_HXL         (0x03)
#define AKM_REG_ST2         (0x09)

#define AKM_REG_CNTL        (0x0A)
#define AKM_REG_ASTC        (0x0C)
#define AKM_REG_ASAX        (0x10)
#define AKM_REG_ASAY        (0x11)
#define AKM_REG_ASAZ        (0x12)

#define AKM_DATA_READY      (0x01)
#define AKM_DATA_OVERRUN    (0x02)
#define AKM_OVERFLOW        (0x80)
#define AKM_DATA_ERROR      (0x40)

#define AKM_BIT_SELF_TEST   (0x40)

#define AKM_POWER_DOWN          (0x00 | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_SINGLE_MEASUREMENT  (0x01 | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_FUSE_ROM_ACCESS     (0x0F | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_MODE_SELF_TEST      (0x08 | SUPPORTS_AK89xx_HIGH_SENS)

#define AKM_WHOAMI      (0x48)
#endif

#ifdef MPU6050
const struct gyro_reg_s reg =
{
	.who_am_i       = 0x75,
	.rate_div       = 0x19,
	.lpf            = 0x1A,
	.prod_id        = 0x0C,
	.user_ctrl      = 0x6A,
	.fifo_en        = 0x23,
	.gyro_cfg       = 0x1B,
	.accel_cfg      = 0x1C,
	.motion_thr     = 0x1F,
	.motion_dur     = 0x20,
	.fifo_count_h   = 0x72,
	.fifo_r_w       = 0x74,
	.raw_gyro       = 0x43,
	.raw_accel      = 0x3B,
	.temp           = 0x41,
	.int_enable     = 0x38,
	.dmp_int_status = 0x39,
	.int_status     = 0x3A,
	.pwr_mgmt_1     = 0x6B,
	.pwr_mgmt_2     = 0x6C,
	.int_pin_cfg    = 0x37,
	.mem_r_w        = 0x6F,
	.accel_offs     = 0x06,
	.i2c_mst        = 0x24,
	.bank_sel       = 0x6D,
	.mem_start_addr = 0x6E,
	.prgm_start_h   = 0x70
#ifdef AK89xx_SECONDARY
	,.raw_compass   = 0x49,
	.yg_offs_tc     = 0x01,
	.s0_addr        = 0x25,
	.s0_reg         = 0x26,
	.s0_ctrl        = 0x27,
	.s1_addr        = 0x28,
	.s1_reg         = 0x29,
	.s1_ctrl        = 0x2A,
	.s4_ctrl        = 0x34,
	.s0_do          = 0x63,
	.s1_do          = 0x64,
	.i2c_delay_ctrl = 0x67
#endif
};
const struct hw_s hw =
{
	.addr           = 0x68,
	.max_fifo       = 1024,
	.num_reg        = 118,
	.temp_sens      = 340,
	.temp_offset    = -521,
	.bank_size      = 256
#ifdef AK89xx_SECONDARY
	,.compass_fsr    = AK89xx_FSR
#endif
};

const struct test_s test =
{
	.gyro_sens      = 32768/250,
	.accel_sens     = 32768/16,
	.reg_rate_div   = 0,    /* 1kHz. */
	.reg_lpf        = 1,    /* 188Hz. */
	.reg_gyro_fsr   = 0,    /* 250dps. */
	.reg_accel_fsr  = 0x18, /* 16g. */
	.wait_ms        = 50,
	.packet_thresh  = 5,    /* 5% */
	.min_dps        = 10.f,
	.max_dps        = 105.f,
	.max_gyro_var   = 0.14f,
	.min_g          = 0.3f,
	.max_g          = 0.95f,
	.max_accel_var  = 0.14f
};

static struct gyro_state_s st =
{
	.reg = &reg,
	.hw = &hw,
	.test = &test
};
#elif defined MPU6500
const struct gyro_reg_s reg =
{
	.who_am_i       = 0x75,
	.rate_div       = 0x19,
	.lpf            = 0x1A,
	.prod_id        = 0x0C,
	.user_ctrl      = 0x6A,
	.fifo_en        = 0x23,
	.gyro_cfg       = 0x1B,
	.accel_cfg      = 0x1C,
	.accel_cfg2     = 0x1D,
	.lp_accel_odr   = 0x1E,
	.motion_thr     = 0x1F,
	.motion_dur     = 0x20,
	.fifo_count_h   = 0x72,
	.fifo_r_w       = 0x74,
	.raw_gyro       = 0x43,
	.raw_accel      = 0x3B,
	.temp           = 0x41,
	.int_enable     = 0x38,
	.dmp_int_status = 0x39,
	.int_status     = 0x3A,
	.accel_intel    = 0x69,
	.pwr_mgmt_1     = 0x6B,
	.pwr_mgmt_2     = 0x6C,
	.int_pin_cfg    = 0x37,
	.mem_r_w        = 0x6F,
	.accel_offs     = 0x77,
	.i2c_mst        = 0x24,
	.bank_sel       = 0x6D,
	.mem_start_addr = 0x6E,
	.prgm_start_h   = 0x70
#ifdef AK89xx_SECONDARY
	,.raw_compass   = 0x49,
	.s0_addr        = 0x25,
	.s0_reg         = 0x26,
	.s0_ctrl        = 0x27,
	.s1_addr        = 0x28,
	.s1_reg         = 0x29,
	.s1_ctrl        = 0x2A,
	.s4_ctrl        = 0x34,
	.s0_do          = 0x63,
	.s1_do          = 0x64,
	.i2c_delay_ctrl = 0x67
#endif
};
const struct hw_s hw =
{
	.addr           = 0x68,
	.max_fifo       = 1024,
	.num_reg        = 128,
	.temp_sens      = 321,
	.temp_offset    = 0,
	.bank_size      = 256
#ifdef AK89xx_SECONDARY
	,.compass_fsr    = AK89xx_FSR
#endif
};

const struct test_s test =
{
	.gyro_sens      = 32768/250,
	.accel_sens     = 32768/16,
	.reg_rate_div   = 0,    /* 1kHz. */
	.reg_lpf        = 1,    /* 188Hz. */
	.reg_gyro_fsr   = 0,    /* 250dps. */
	.reg_accel_fsr  = 0x18, /* 16g. */
	.wait_ms        = 50,
	.packet_thresh  = 5,    /* 5% */
	.min_dps        = 10.f,
	.max_dps        = 105.f,
	.max_gyro_var   = 0.14f,
	.min_g          = 0.3f,
	.max_g          = 0.95f,
	.max_accel_var  = 0.14f
};

static struct gyro_state_s st =
{
	.reg = &reg,
	.hw = &hw,
	.test = &test
};
#endif

#define MAX_PACKET_LENGTH (12)

#ifdef AK89xx_SECONDARY
static int setup_compass(void);
#define MAX_COMPASS_SAMPLE_RATE (100)
#endif

static unsigned int get_word_measurement(unsigned char r_addr)
{
    unsigned int value;
    value = i2c_smbus_read_word_data(fd_mpu6050, r_addr);

    return (value);
}


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


/**
 *  @brief      Enable/disable data ready interrupt.
 *  If the DMP is on, the DMP interrupt is enabled. Otherwise, the data ready
 *  interrupt is used.
 *  @param[in]  enable      1 to enable interrupt.
 *  @return     0 if successful.
 */
static int set_int_enable(u_int8 enable)
{
	u_int8 tmp;

	if (st.chip_cfg.dmp_on)
	{
		if (enable)
			tmp = BIT_DMP_INT_EN;
		else
			tmp = 0x00;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, tmp)<0)
			return 1;
		st.chip_cfg.int_enable = tmp;
	}
	else
	{
		if (!st.chip_cfg.sensors)
			return 1;
		if (enable && st.chip_cfg.int_enable)
			return 0;
		if (enable)
			tmp = BIT_DATA_RDY_EN;
		else
			tmp = 0x00;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, tmp)<0)
			return 1;
		st.chip_cfg.int_enable = tmp;
	}
	return 0;
}

/**
 *  @brief      Register dump for testing.
 *  @return     0 if successful.
 */
u_int8 mpu_reg_dump(void)
{
	u_int8 ii;
	u_int8 data;

	for (ii = 0; ii < st.hw->num_reg; ii++)
	{
		if (ii == st.reg->fifo_r_w || ii == st.reg->mem_r_w)
			continue;
		if((data = i2c_smbus_read_byte_data(fd_mpu6050, ii))<0)
            return 1;
#ifdef MPU_DEBUG
		printf("%#5x: %#5x\r\r\n", ii, data);
#endif
	}
	return 0;
}

/**
 *  @brief      Read from a single register.
 *  NOTE: The memory and FIFO read/write registers cannot be accessed.
 *  @param[in]  reg     Register address.
 *  @param[out] data    Register data.
 *  @return     0 if successful.
 */
u_int8 mpu_read_reg(u_int8 reg, u_int8 *data)
{
    int value;
	if (reg == st.reg->fifo_r_w || reg == st.reg->mem_r_w)
		return 1;
	if (reg >= st.hw->num_reg)
		return 1;
	if((*data = i2c_smbus_read_byte_data(fd_mpu6050, reg))<0)
        return 1;
    return 0;
}

/**
 *  @brief      Initialize hardware.
 *  Initial configuration:\n
 *  Gyro FSR: +/- 2000DPS\n
 *  Accel FSR +/- 2G\n
 *  DLPF: 42Hz\n
 *  FIFO rate: 50Hz\n
 *  Clock source: Gyro PLL\n
 *  FIFO: Disabled.\n
 *  Data ready interrupt: Disabled, active low, unlatched.
 *  @param[in]  int_param   Platform-specific parameters to interrupt API.
 *  @return     0 if successful.
 */
u_int8 mpu_init(struct int_param_s *int_param)
{
	u_int8 data[6], rev;

	mpu6050_init();

	/* Reset device. */
	data[0] = BIT_RESET;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, data[0])<0)
		return 1;
	delay_ms(100);

	/* Wake up chip. */
	data[0] = 0x00;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, data[0])<0)
		return 1;

#ifdef MPU6050
	/* Check product revision. */
	if (i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->accel_offs, 6, data)<0)
		return 1;
	rev = ((data[5] & 0x01) << 2) | ((data[3] & 0x01) << 1) |
				(data[1] & 0x01);
#ifdef MPU_DEBUG
	printf("Software product rev. %d.\r\n", rev);
#endif
	if (rev)
	{
		/* Congrats, these parts are better. */
		if (rev == 1)
			st.chip_cfg.accel_half = 1;
		else if (rev == 2)
			st.chip_cfg.accel_half = 0;
		else
		{
#ifdef MPU_DEBUG
			printf("Unsupported software product rev. %d.\r\n", rev);
#endif
			return 1;
		}
	}
	else
	{
		if ((data[0]=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->prod_id)) < 0)
			return 1;
		rev = data[0] & 0x0F;
		if (!rev)
		{
#ifdef MPU_DEBUG
			printf("Product ID read as 0 indicates device is either "
						"incompatible or an MPU3050.\r\n");
#endif
			return 1;
		}
		else if (rev == 4)
		{
#ifdef MPU_DEBUG
			printf("Half sensitivity part found.\r\n");
#endif
			st.chip_cfg.accel_half = 1;
		}
		else
			st.chip_cfg.accel_half = 0;
	}
#elif defined MPU6500
#define MPU6500_MEM_REV_ADDR    (0x17)
	if (mpu_read_mem(MPU6500_MEM_REV_ADDR, &rev))
		return 1;
	if (rev == 0x1)
		st.chip_cfg.accel_half = 0;
	else
	{
#ifdef MPU_DEBUG
		printf("Unsupported software product rev. %d.\r\n", rev);
#endif
		return 1;
	}

	/* MPU6500 shares 4kB of memory between the DMP and the FIFO. Since the
	 * first 3kB are needed by the DMP, we'll use the last 1kB for the FIFO.
	 */
	data[0] = BIT_FIFO_SIZE_1024 | 0x8;
	if (i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_cfg2, data)!=1)
		return 1;
#endif

	/* Set to invalid values to ensure no I2C writes are skipped. */
	st.chip_cfg.sensors = 0xFF;
	st.chip_cfg.gyro_fsr = 0xFF;
	st.chip_cfg.accel_fsr = 0xFF;
	st.chip_cfg.lpf = 0xFF;
	st.chip_cfg.sample_rate = 0xFFFF;
	st.chip_cfg.fifo_enable = 0xFF;
	st.chip_cfg.bypass_mode = 0xFF;
#ifdef AK89xx_SECONDARY
	st.chip_cfg.compass_sample_rate = 0xFFFF;
#endif
	/* mpu_set_sensors always preserves this setting. */
	st.chip_cfg.clk_src = INV_CLK_PLL;
	/* Handled in next call to mpu_set_bypass. */
	st.chip_cfg.active_low_int = 1;
	st.chip_cfg.latched_int = 0;
	st.chip_cfg.int_motion_only = 0;
	st.chip_cfg.lp_accel_mode = 0;
	memset(&st.chip_cfg.cache, 0, sizeof(st.chip_cfg.cache));
	st.chip_cfg.dmp_on = 0;
	st.chip_cfg.dmp_loaded = 0;
	st.chip_cfg.dmp_sample_rate = 0;

	if (mpu_set_gyro_fsr(2000))
		return 1;
	if (mpu_set_accel_fsr(2))
		return 1;
	if (mpu_set_lpf(42))
		return 1;
	if (mpu_set_sample_rate(50))
		return 1;
	if (mpu_configure_fifo(0))
		return 1;

	/* if (int_param)
	    reg_int_cb(int_param); */

#ifdef AK89xx_SECONDARY
	setup_compass();
	if (mpu_set_compass_sample_rate(10))
		return 1;
#else
	/* Already disabled by setup_compass. */
	if (mpu_set_bypass(0))
		return 1;
#endif

	mpu_set_sensors(0);
#ifdef MPU_DEBUG
	printf("Initializing is done...\r\n");
#endif
	return 0;
}

/**
 *  @brief      Enter low-power accel-only mode.
 *  In low-power accel mode, the chip goes to sleep and only wakes up to sample
 *  the accelerometer at one of the following frequencies:
 *  \n MPU6050: 1.25Hz, 5Hz, 20Hz, 40Hz
 *  \n MPU6500: 1.25Hz, 2.5Hz, 5Hz, 10Hz, 20Hz, 40Hz, 80Hz, 160Hz, 320Hz, 640Hz
 *  \n If the requested rate is not one listed above, the device will be set to
 *  the next highest rate. Requesting a rate above the maximum supported
 *  frequency will result in an error.
 *  \n To select a fractional wake-up frequency, round down the value passed to
 *  @e rate.
 *  @param[in]  rate        Minimum sampling rate, or zero to disable LP
 *                          accel mode.
 *  @return     0 if successful.
 */
u_int8 mpu_lp_accel_mode(u_int8 rate)
{
	u_int8 tmp[2];

	if (rate > 40)
		return 1;

	if (!rate)
	{
		mpu_set_int_latched(0);
		tmp[0] = 0;
		tmp[1] = BIT_STBY_XYZG;
		if (i2c_smbus_write_word_data(fd_mpu6050, st.reg->pwr_mgmt_1, tmp)!=2)
			return 1;
		st.chip_cfg.lp_accel_mode = 0;
		return 0;
	}
	/* For LP accel, we automatically configure the hardware to produce latched
	 * interrupts. In LP accel mode, the hardware cycles into sleep mode before
	 * it gets a chance to deassert the interrupt pin; therefore, we shift this
	 * responsibility over to the MCU.
	 *
	 * Any register read will clear the interrupt.
	 */
	mpu_set_int_latched(1);
#ifdef MPU6050
	tmp[0] = BIT_LPA_CYCLE;
	if (rate == 1)
	{
		tmp[1] = INV_LPA_1_25HZ;
		mpu_set_lpf(5);
	}
	else if (rate <= 5)
	{
		tmp[1] = INV_LPA_5HZ;
		mpu_set_lpf(5);
	}
	else if (rate <= 20)
	{
		tmp[1] = INV_LPA_20HZ;
		mpu_set_lpf(10);
	}
	else
	{
		tmp[1] = INV_LPA_40HZ;
		mpu_set_lpf(20);
	}
	tmp[1] = (tmp[1] << 6) | BIT_STBY_XYZG;
	if (i2c_smbus_write_word_data(fd_mpu6050, st.reg->pwr_mgmt_1, tmp)!=2)
		return 1;
#elif defined MPU6500
	/* Set wake frequency. */
	if (rate == 1)
		tmp[0] = INV_LPA_1_25HZ;
	else if (rate == 2)
		tmp[0] = INV_LPA_2_5HZ;
	else if (rate <= 5)
		tmp[0] = INV_LPA_5HZ;
	else if (rate <= 10)
		tmp[0] = INV_LPA_10HZ;
	else if (rate <= 20)
		tmp[0] = INV_LPA_20HZ;
	else if (rate <= 40)
		tmp[0] = INV_LPA_40HZ;
	else if (rate <= 80)
		tmp[0] = INV_LPA_80HZ;
	else if (rate <= 160)
		tmp[0] = INV_LPA_160HZ;
	else if (rate <= 320)
		tmp[0] = INV_LPA_320HZ;
	else
		tmp[0] = INV_LPA_640HZ;
	if (i2c_smbus_write_byte_data(fd_mpu6050, st.reg->lp_accel_odr, tmp)!=1)
		return 1;
	tmp[0] = BIT_LPA_CYCLE;
	if (i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, tmp)!=1)
		return 1;
#endif
	st.chip_cfg.sensors = INV_XYZ_ACCEL;
	st.chip_cfg.clk_src = 0;
	st.chip_cfg.lp_accel_mode = 1;
	mpu_configure_fifo(0);

	return 0;
}

/**
 *  @brief      Read raw gyro data directly from the registers.
 *  @param[out] data        Raw data in hardware units.
 *  @return     0 if successful.
 */
u_int8 mpu_get_gyro_reg(int16_t *data)
{
	u_int8 tmp[6];

	if (!(st.chip_cfg.sensors & INV_XYZ_GYRO))
		return 1;

	if(i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->raw_gyro, 6, tmp)<0)
        return 1;
	data[0] = (tmp[0] << 8) | tmp[1];
	data[1] = (tmp[2] << 8) | tmp[3];
	data[2] = (tmp[4] << 8) | tmp[5];
	return 0;
}

/**
 *  @brief      Read raw accel data directly from the registers.
 *  @param[out] data        Raw data in hardware units.
 *  @return     0 if successful.
 */
u_int8 mpu_get_accel_reg(int16_t *data)
{
	u_int8 tmp[6];

	if (!(st.chip_cfg.sensors & INV_XYZ_ACCEL))
		return 1;

	if(i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->raw_accel, 6, tmp)<0)
        return 1;
	data[0] = (tmp[0] << 8) | tmp[1];
	data[1] = (tmp[2] << 8) | tmp[3];
	data[2] = (tmp[4] << 8) | tmp[5];
	return 0;
}

/**
 *  @brief      Read temperature data directly from the registers.
 *  @param[out] data        Data in q16 format.
 *  @return     0 if successful.
 */
u_int8 mpu_get_temperature(int32_t *data)
{
	int16_t raw;

	if (!(st.chip_cfg.sensors))
		return 1;

    raw = get_word_measurement(st.reg->temp);

	data[0] = (int32_t)((35 + ((raw - (float)st.hw->temp_offset) / st.hw->temp_sens)) * 65536L);
	return 0;
}

/**
 *  @brief      Push biases to the accel bias registers.
 *  This function expects biases relative to the current sensor output, and
 *  these biases will be added to the factory-supplied values.
 *  @param[in]  accel_bias  New biases.
 *  @return     0 if successful.
 */
u_int8 mpu_set_accel_bias(const int32_t *accel_bias)
{
	u_int8 data[6];
	int16_t accel_hw[3];
	int16_t got_accel[3];
	int16_t fg[3];

	if (!accel_bias)
		return 1;
	if (!accel_bias[0] && !accel_bias[1] && !accel_bias[2])
		return 0;

    i2c_smbus_read_i2c_block_data(fd_mpu6050, 3, 3, data);
	fg[0] = ((data[0] >> 4) + 8) & 0xf;
	fg[1] = ((data[1] >> 4) + 8) & 0xf;
	fg[2] = ((data[2] >> 4) + 8) & 0xf;

	accel_hw[0] = (int16_t)(accel_bias[0] * 2 / (64 + fg[0]));
	accel_hw[1] = (int16_t)(accel_bias[1] * 2 / (64 + fg[1]));
	accel_hw[2] = (int16_t)(accel_bias[2] * 2 / (64 + fg[2]));

	i2c_smbus_read_i2c_block_data(fd_mpu6050, 0x06, 6, data);
	got_accel[0] = ((int16_t)data[0] << 8) | data[1];
	got_accel[1] = ((int16_t)data[2] << 8) | data[3];
	got_accel[2] = ((int16_t)data[4] << 8) | data[5];

	accel_hw[0] += got_accel[0];
	accel_hw[1] += got_accel[1];
	accel_hw[2] += got_accel[2];

	data[0] = (accel_hw[0] >> 8) & 0xff;
	data[1] = (accel_hw[0]) & 0xff;
	data[2] = (accel_hw[1] >> 8) & 0xff;
	data[3] = (accel_hw[1]) & 0xff;
	data[4] = (accel_hw[2] >> 8) & 0xff;
	data[5] = (accel_hw[2]) & 0xff;

	i2c_smbus_write_block_data(fd_mpu6050, 0x06, 6, data);
	return 0;
}

/**
 *  @brief  Reset FIFO read/write pointers.
 *  @return 0 if successful.
 */
u_int8 mpu_reset_fifo(void)
{
	u_int8 data;

	if (!(st.chip_cfg.sensors))
		return 1;

	data = 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data)<0)
        return 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, data)<0)
        return 1;
    if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data)<0)
        return 1;

	if (st.chip_cfg.dmp_on)
	{
		data = BIT_FIFO_RST | BIT_DMP_RST;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data)<0)
            return 1;
		delay_ms(50);
		data = BIT_DMP_EN | BIT_FIFO_EN;
		if (st.chip_cfg.sensors & INV_XYZ_COMPASS)
			data |= BIT_AUX_IF_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data)<0)
            return 1;
		if (st.chip_cfg.int_enable)
			data = BIT_DMP_INT_EN;
		else
			data = 0;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data)<0)
            return 1;
		data = 0;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, data)<0)
			return 1;
	}
	else
	{
		data = BIT_FIFO_RST;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data)<0)
            return 1;

		if (st.chip_cfg.bypass_mode || !(st.chip_cfg.sensors & INV_XYZ_COMPASS))
			data = BIT_FIFO_EN;
		else
			data = BIT_FIFO_EN | BIT_AUX_IF_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data)<0)
            return 1;

		delay_ms(50);
		if (st.chip_cfg.int_enable)
			data = BIT_DATA_RDY_EN;
		else
			data = 0;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data)<0)
            return 1;

		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, st.chip_cfg.fifo_enable)<0)
            return 1;
		
	}
	return 0;
}

/**
 *  @brief      Get the gyro full-scale range.
 *  @param[out] fsr Current full-scale range.
 *  @return     0 if successful.
 */
u_int8 mpu_get_gyro_fsr(u_int16 *fsr)
{
	switch (st.chip_cfg.gyro_fsr)
	{
	    case INV_FSR_250DPS:
		    fsr[0] = 250;
		    break;
	    case INV_FSR_500DPS:
		    fsr[0] = 500;
		    break;
	    case INV_FSR_1000DPS:
		    fsr[0] = 1000;
		    break;
	    case INV_FSR_2000DPS:
		    fsr[0] = 2000;
		    break;
	    default:
		    fsr[0] = 0;
		    break;
	}
	return 0;
}

/**
 *  @brief      Set the gyro full-scale range.
 *  @param[in]  fsr Desired full-scale range.
 *  @return     0 if successful.
 */
u_int8 mpu_set_gyro_fsr(u_int16 fsr)
{
	u_int8 data;

	if (!(st.chip_cfg.sensors))
		return 1;

	switch (fsr)
	{
	    case 250:
		    data = INV_FSR_250DPS << 3;
		    break;
	    case 500:
		    data = INV_FSR_500DPS << 3;
		    break;
	    case 1000:
		    data = INV_FSR_1000DPS << 3;
		    break;
	    case 2000:
		    data = INV_FSR_2000DPS << 3;
		    break;
	    default:
		    return 1;
	}

	if (st.chip_cfg.gyro_fsr == (data >> 3))
		return 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->gyro_cfg, data)<0)
        return 1;

	st.chip_cfg.gyro_fsr = data >> 3;
	return 0;
}

/**
 *  @brief      Get the accel full-scale range.
 *  @param[out] fsr Current full-scale range.
 *  @return     0 if successful.
 */
u_int8 mpu_get_accel_fsr(u_int8 *fsr)
{
	switch (st.chip_cfg.accel_fsr)
	{
	    case INV_FSR_2G:
		    fsr[0] = 2;
		    break;
	    case INV_FSR_4G:
		    fsr[0] = 4;
		    break;
	    case INV_FSR_8G:
		    fsr[0] = 8;
		    break;
	    case INV_FSR_16G:
		    fsr[0] = 16;
		    break;
	    default:
		    return 1;
	}
	if (st.chip_cfg.accel_half)
		fsr[0] <<= 1;
	return 0;
}

/**
 *  @brief      Set the accel full-scale range.
 *  @param[in]  fsr Desired full-scale range.
 *  @return     0 if successful.
 */
u_int8 mpu_set_accel_fsr(u_int8 fsr)
{
	u_int8 data;

	if (!(st.chip_cfg.sensors))
		return 1;

	switch (fsr)
	{
	    case 2:
		    data = INV_FSR_2G << 3;
		    break;
	    case 4:
		    data = INV_FSR_4G << 3;
		    break;
	    case 8:
		    data = INV_FSR_8G << 3;
		    break;
	    case 16:
		    data = INV_FSR_16G << 3;
		    break;
	    default:
		    return 1;
	}

	if (st.chip_cfg.accel_fsr == (data >> 3))
		return 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_cfg, data)<0)
	    return 1;

	st.chip_cfg.accel_fsr = data >> 3;
	return 0;
}

/**
 *  @brief      Get the current DLPF setting.
 *  @param[out] lpf Current LPF setting.
 *  0 if successful.
 */
u_int8 mpu_get_lpf(u_int16 *lpf)
{
	switch (st.chip_cfg.lpf)
	{
	    case INV_FILTER_188HZ:
		    lpf[0] = 188;
		    break;
	    case INV_FILTER_98HZ:
		    lpf[0] = 98;
		    break;
	    case INV_FILTER_42HZ:
		    lpf[0] = 42;
		    break;
	    case INV_FILTER_20HZ:
		    lpf[0] = 20;
		    break;
	    case INV_FILTER_10HZ:
		    lpf[0] = 10;
		    break;
	    case INV_FILTER_5HZ:
		    lpf[0] = 5;
		    break;
	    case INV_FILTER_256HZ_NOLPF2:
	    case INV_FILTER_2100HZ_NOLPF:
	    default:
		    lpf[0] = 0;
		    break;
	}
	return 0;
}

/**
 *  @brief      Set digital low pass filter.
 *  The following LPF settings are supported: 188, 98, 42, 20, 10, 5.
 *  @param[in]  lpf Desired LPF setting.
 *  @return     0 if successful.
 */
u_int8 mpu_set_lpf(u_int16 lpf)
{
	u_int8 data;

	if (!(st.chip_cfg.sensors))
		return 1;

	if (lpf >= 188)
		data = INV_FILTER_188HZ;
	else if (lpf >= 98)
		data = INV_FILTER_98HZ;
	else if (lpf >= 42)
		data = INV_FILTER_42HZ;
	else if (lpf >= 20)
		data = INV_FILTER_20HZ;
	else if (lpf >= 10)
		data = INV_FILTER_10HZ;
	else
		data = INV_FILTER_5HZ;

	if (st.chip_cfg.lpf == data)
		return 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->lpf, data)<0)
        return 1;

	st.chip_cfg.lpf = data;
	return 0;
}

/**
 *  @brief      Get sampling rate.
 *  @param[out] rate    Current sampling rate (Hz).
 *  @return     0 if successful.
 */
u_int8 mpu_get_sample_rate(u_int16 *rate)
{
	if (st.chip_cfg.dmp_on)
		return 1;
	else
		rate[0] = st.chip_cfg.sample_rate;
	return 0;
}

/**
 *  @brief      Set sampling rate.
 *  Sampling rate must be between 4Hz and 1kHz.
 *  @param[in]  rate    Desired sampling rate (Hz).
 *  @return     0 if successful.
 */
u_int8 mpu_set_sample_rate(u_int16 rate)
{
	u_int8 data;

	if (!(st.chip_cfg.sensors))
		return 1;

	if (st.chip_cfg.dmp_on)
		return 1;
	else
	{
		if (st.chip_cfg.lp_accel_mode)
		{
			if (rate && (rate <= 40))
			{
				/* Just stay in low-power accel mode. */
				mpu_lp_accel_mode(rate);
				return 0;
			}
			/* Requested rate exceeds the allowed frequencies in LP accel mode,
			 * switch back to full-power mode.
			 */
			mpu_lp_accel_mode(0);
		}
		if (rate < 4)
			rate = 4;
		else if (rate > 1000)
			rate = 1000;

		data = 1000 / rate - 1;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->rate_div, data)<0)
            return 1;

		st.chip_cfg.sample_rate = 1000 / (1 + data);

#ifdef AK89xx_SECONDARY
		mpu_set_compass_sample_rate(min(st.chip_cfg.compass_sample_rate, MAX_COMPASS_SAMPLE_RATE));
#endif

		/* Automatically set LPF to 1/2 sampling rate. */
		mpu_set_lpf(st.chip_cfg.sample_rate >> 1);
		return 0;
	}
}

/**
 *  @brief      Get compass sampling rate.
 *  @param[out] rate    Current compass sampling rate (Hz).
 *  @return     0 if successful.
 */
u_int8 mpu_get_compass_sample_rate(u_int16 *rate)
{
#ifdef AK89xx_SECONDARY
	rate[0] = st.chip_cfg.compass_sample_rate;
	return 0;
#else
	rate[0] = 0;
	return 1;
#endif
}

/**
 *  @brief      Set compass sampling rate.
 *  The compass on the auxiliary I2C bus is read by the MPU hardware at a
 *  maximum of 100Hz. The actual rate can be set to a fraction of the gyro
 *  sampling rate.
 *
 *  \n WARNING: The new rate may be different than what was requested. Call
 *  mpu_get_compass_sample_rate to check the actual setting.
 *  @param[in]  rate    Desired compass sampling rate (Hz).
 *  @return     0 if successful.
 */
u_int8 mpu_set_compass_sample_rate(u_int16 rate)
{
#ifdef AK89xx_SECONDARY
	u_int8 div;
	if (!rate || rate > st.chip_cfg.sample_rate || rate > MAX_COMPASS_SAMPLE_RATE)
		return 1;

	div = st.chip_cfg.sample_rate / rate - 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s4_ctrl, div)<0)
        return 1;
	
	st.chip_cfg.compass_sample_rate = st.chip_cfg.sample_rate / (div + 1);
	return 0;
#else
	return 1;
#endif
}

/**
 *  @brief      Get gyro sensitivity scale factor.
 *  @param[out] sens    Conversion from hardware units to dps.
 *  @return     0 if successful.
 */
u_int8 mpu_get_gyro_sens(float *sens)
{
	switch (st.chip_cfg.gyro_fsr)
	{
	    case INV_FSR_250DPS:
		    sens[0] = 131.f;
		    break;
	    case INV_FSR_500DPS:
		    sens[0] = 65.5f;
		    break;
	    case INV_FSR_1000DPS:
		    sens[0] = 32.8f;
		    break;
	    case INV_FSR_2000DPS:
		    sens[0] = 16.4f;
		    break;
	    default:
		    return 1;
	}
	return 0;
}

/**
 *  @brief      Get accel sensitivity scale factor.
 *  @param[out] sens    Conversion from hardware units to g's.
 *  @return     0 if successful.
 */
u_int8 mpu_get_accel_sens(u_int16 *sens)
{
	switch (st.chip_cfg.accel_fsr)
	{
	    case INV_FSR_2G:
		    sens[0] = 16384;
		    break;
	    case INV_FSR_4G:
		    sens[0] = 8092;
		    break;
	    case INV_FSR_8G:
		    sens[0] = 4096;
		    break;
	    case INV_FSR_16G:
		    sens[0] = 2048;
		    break;
	    default:
		    return 1;
	}
	if (st.chip_cfg.accel_half)
		sens[0] >>= 1;
	return 0;
}

/**
 *  @brief      Get current FIFO configuration.
 *  @e sensors can contain a combination of the following flags:
 *  \n INV_X_GYRO, INV_Y_GYRO, INV_Z_GYRO
 *  \n INV_XYZ_GYRO
 *  \n INV_XYZ_ACCEL
 *  @param[out] sensors Mask of sensors in FIFO.
 *  @return     0 if successful.
 */
u_int8 mpu_get_fifo_config(u_int8 *sensors)
{
	sensors[0] = st.chip_cfg.fifo_enable;
	return 0;
}

/**
 *  @brief      Select which sensors are pushed to FIFO.
 *  @e sensors can contain a combination of the following flags:
 *  \n INV_X_GYRO, INV_Y_GYRO, INV_Z_GYRO
 *  \n INV_XYZ_GYRO
 *  \n INV_XYZ_ACCEL
 *  @param[in]  sensors Mask of sensors to push to FIFO.
 *  @return     0 if successful.
 */
u_int8 mpu_configure_fifo(u_int8 sensors)
{
	u_int8 prev;
	int result = 0;

	/* Compass data isn't going into the FIFO. Stop trying. */
	sensors &= ~INV_XYZ_COMPASS;

	if (st.chip_cfg.dmp_on)
		return 0;
	else
	{
		if (!(st.chip_cfg.sensors))
			return 1;
		prev = st.chip_cfg.fifo_enable;
		st.chip_cfg.fifo_enable = sensors & st.chip_cfg.sensors;
		if (st.chip_cfg.fifo_enable != sensors)
			/* You're not getting what you asked for. Some sensors are
			 * asleep.
			 */
			result = -1;
		else
			result = 0;
		if (sensors || st.chip_cfg.lp_accel_mode)
			set_int_enable(1);
		else
			set_int_enable(0);
		if (sensors)
		{
			if (mpu_reset_fifo())
			{
				st.chip_cfg.fifo_enable = prev;
				return 1;
			}
		}
	}

	return result;
}

/**
 *  @brief      Get current power state.
 *  @param[in]  power_on    1 if turned on, 0 if suspended.
 *  @return     0 if successful.
 */
u_int8 mpu_get_power_state(u_int8 *power_on)
{
	if (st.chip_cfg.sensors)
		power_on[0] = 1;
	else
		power_on[0] = 0;
	return 0;
}

/**
 *  @brief      Turn specific sensors on/off.
 *  @e sensors can contain a combination of the following flags:
 *  \n INV_X_GYRO, INV_Y_GYRO, INV_Z_GYRO
 *  \n INV_XYZ_GYRO
 *  \n INV_XYZ_ACCEL
 *  \n INV_XYZ_COMPASS
 *  @param[in]  sensors    Mask of sensors to wake.
 *  @return     0 if successful.
 */
u_int8 mpu_set_sensors(u_int8 sensors)
{
	u_int8 data;
#ifdef AK89xx_SECONDARY
	u_int8 user_ctrl;
#endif

	if (sensors & INV_XYZ_GYRO)
		data = INV_CLK_PLL;
	else if (sensors)
		data = 0;
	else
		data = BIT_SLEEP;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, data)<0)
        return 1;

	st.chip_cfg.clk_src = data & ~BIT_SLEEP;

	data = 0;
	if (!(sensors & INV_X_GYRO))
		data |= BIT_STBY_XG;
	if (!(sensors & INV_Y_GYRO))
		data |= BIT_STBY_YG;
	if (!(sensors & INV_Z_GYRO))
		data |= BIT_STBY_ZG;
	if (!(sensors & INV_XYZ_ACCEL))
		data |= BIT_STBY_XYZA;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_2, data)<0)
        return 1;

	if (sensors && (sensors != INV_XYZ_ACCEL))
		/* Latched interrupts only used in LP accel mode. */
		mpu_set_int_latched(0);

#ifdef AK89xx_SECONDARY
#ifdef AK89xx_BYPASS
	if (sensors & INV_XYZ_COMPASS)
		mpu_set_bypass(1);
	else
		mpu_set_bypass(0);
#else
	if((user_ctrl=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->user_ctrl))<0)
        return 1;
	
	/* Handle AKM power management. */
	if (sensors & INV_XYZ_COMPASS)
	{
		data = AKM_SINGLE_MEASUREMENT;
		user_ctrl |= BIT_AUX_IF_EN;
	}
	else
	{
		data = AKM_POWER_DOWN;
		user_ctrl &= ~BIT_AUX_IF_EN;
	}
	if (st.chip_cfg.dmp_on)
		user_ctrl |= BIT_DMP_EN;
	else
		user_ctrl &= ~BIT_DMP_EN;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s1_do, data)<0)
        return 1;
	
	/* Enable/disable I2C master mode. */
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, user_ctrl)<0)
        return 1;
#endif
#endif

	st.chip_cfg.sensors = sensors;
	st.chip_cfg.lp_accel_mode = 0;
	delay_ms(50);
	return 0;
}

/**
 *  @brief      Read the MPU interrupt status registers.
 *  @param[out] status  Mask of interrupt bits.
 *  @return     0 if successful.
 */
u_int8 mpu_get_int_status(int16_t *status)
{
	if (!st.chip_cfg.sensors)
		return 1;
	if((*status=i2c_smbus_read_word_data(fd_mpu6050, st.reg->dmp_int_status))<0)
        return 1;

	return 0;
}

/**
 *  @brief      Get one packet from the FIFO.
 *  If @e sensors does not contain a particular sensor, disregard the data
 *  returned to that pointer.
 *  \n @e sensors can contain a combination of the following flags:
 *  \n INV_X_GYRO, INV_Y_GYRO, INV_Z_GYRO
 *  \n INV_XYZ_GYRO
 *  \n INV_XYZ_ACCEL
 *  \n If the FIFO has no new data, @e sensors will be zero.
 *  \n If the FIFO is disabled, @e sensors will be zero and this function will
 *  return a non-zero error code.
 *  @param[out] gyro        Gyro data in hardware units.
 *  @param[out] accel       Accel data in hardware units.
 *  @param[out] sensors     Mask of sensors read from FIFO.
 *  @param[out] more        Number of remaining packets.
 *  @return     0 if successful.
 */
u_int8 mpu_read_fifo(int16_t *gyro, int16_t *accel,
											u_int8 *sensors, u_int8 *more)
{
	/* Assumes maximum packet size is gyro (6) + accel (6). */
	u_int8 data[MAX_PACKET_LENGTH];
	u_int8 packet_size = 0;
	u_int16 fifo_count, index = 0;

	if (st.chip_cfg.dmp_on)
		return 1;

	sensors[0] = 0;
	if (!st.chip_cfg.sensors)
		return 1;
	if (!st.chip_cfg.fifo_enable)
		return 1;

	if (st.chip_cfg.fifo_enable & INV_X_GYRO)
		packet_size += 2;
	if (st.chip_cfg.fifo_enable & INV_Y_GYRO)
		packet_size += 2;
	if (st.chip_cfg.fifo_enable & INV_Z_GYRO)
		packet_size += 2;
	if (st.chip_cfg.fifo_enable & INV_XYZ_ACCEL)
		packet_size += 6;

	if((fifo_count=i2c_smbus_read_word_data(fd_mpu6050, st.reg->fifo_count_h))<0)
        return 1;
	
	if (fifo_count < packet_size)
		return 0;
#ifdef MPU_DEBUG
    printf("FIFO count: %hd\r\n", fifo_count);
#endif
	if (fifo_count > (st.hw->max_fifo >> 1))
	{
		/* FIFO is 50% full, better check overflow bit. */
		if((data[0]=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->int_status))<0)
            return 1;
		
		if (data[0] & BIT_FIFO_OVERFLOW)
		{
			mpu_reset_fifo();
			return 2;
		}
	}

	i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->fifo_r_w, packet_size, data);

	more[0] = fifo_count / packet_size - 1;
	sensors[0] = 0;

	if ((index != packet_size) && st.chip_cfg.fifo_enable & INV_XYZ_ACCEL)
	{
		accel[0] = (data[index+0] << 8) | data[index+1];
		accel[1] = (data[index+2] << 8) | data[index+3];
		accel[2] = (data[index+4] << 8) | data[index+5];
		sensors[0] |= INV_XYZ_ACCEL;
		index += 6;
	}
	if ((index != packet_size) && st.chip_cfg.fifo_enable & INV_X_GYRO)
	{
		gyro[0] = (data[index+0] << 8) | data[index+1];
		sensors[0] |= INV_X_GYRO;
		index += 2;
	}
	if ((index != packet_size) && st.chip_cfg.fifo_enable & INV_Y_GYRO)
	{
		gyro[1] = (data[index+0] << 8) | data[index+1];
		sensors[0] |= INV_Y_GYRO;
		index += 2;
	}
	if ((index != packet_size) && st.chip_cfg.fifo_enable & INV_Z_GYRO)
	{
		gyro[2] = (data[index+0] << 8) | data[index+1];
		sensors[0] |= INV_Z_GYRO;
		index += 2;
	}

	return 0;
}

/**
 *  @brief      Get one unparsed packet from the FIFO.
 *  This function should be used if the packet is to be parsed elsewhere.
 *  @param[in]  length  Length of one FIFO packet.
 *  @param[in]  data    FIFO packet.
 *  @param[in]  more    Number of remaining packets.
 */
u_int8 mpu_read_fifo_stream(u_int16 length, u_int8 *data,u_int8 *more)
{
	u_int8 tmp[2];
	u_int16 fifo_count;
	if (!st.chip_cfg.dmp_on)
		return 1;
	if (!st.chip_cfg.sensors)
		return 1;

	if((fifo_count=i2c_smbus_read_word_data(fd_mpu6050, st.reg->fifo_count_h))<0)
		return 1;
	
	if (fifo_count < length)
	{
		more[0] = 0;
		return 1;
	}
	if (fifo_count > (st.hw->max_fifo >> 1))
	{
		/* FIFO is 50% full, better check overflow bit. */
		if((tmp[0]=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->int_status))<0)
			return 1;
		
		if (tmp[0] & BIT_FIFO_OVERFLOW)
		{
			mpu_reset_fifo();
			return 2;
		}
	}

	i2c_smbus_read_word_data(fd_mpu6050, st.reg->fifo_r_w, length, data);
	
	more[0] = fifo_count / length - 1;
	return 0;
}

/**
 *  @brief      Set device to bypass mode.
 *  @param[in]  bypass_on   1 to enable bypass mode.
 *  @return     0 if successful.
 */
u_int8 mpu_set_bypass(u_int8 bypass_on)
{
	u_int8 tmp;

	if (st.chip_cfg.bypass_mode == bypass_on)
		return 0;

	if (bypass_on)
	{
		if((tmp=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->user_ctrl))<0)
            return 1;
		
		tmp &= ~BIT_AUX_IF_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, tmp)<0)
            return 1;

		delay_ms(3);
		tmp = BIT_BYPASS_EN;
		if (st.chip_cfg.active_low_int)
			tmp |= BIT_ACTL;
		if (st.chip_cfg.latched_int)
			tmp |= BIT_LATCH_EN | BIT_ANY_RD_CLR;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_pin_cfg, tmp)<0)
            return 1;
		
	}
	else
	{
		/* Enable I2C master mode if compass is being used. */
		if((tmp=i2c_smbus_read_byte_data(fd_mpu6050, st.reg->user_ctrl))<0)
            return 1;
		
		if (st.chip_cfg.sensors & INV_XYZ_COMPASS)
			tmp |= BIT_AUX_IF_EN;
		else
			tmp &= ~BIT_AUX_IF_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, tmp)<0)
            return 1;
		
		delay_ms(3);
		if (st.chip_cfg.active_low_int)
			tmp = BIT_ACTL;
		else
			tmp = 0;
		if (st.chip_cfg.latched_int)
			tmp |= BIT_LATCH_EN | BIT_ANY_RD_CLR;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_pin_cfg, tmp)<0)
            return 1;
		
	}
	st.chip_cfg.bypass_mode = bypass_on;
	return 0;
}

/**
 *  @brief      Set interrupt level.
 *  @param[in]  active_low  1 for active low, 0 for active high.
 *  @return     0 if successful.
 */
u_int8 mpu_set_int_level(u_int8 active_low)
{
	st.chip_cfg.active_low_int = active_low;
	return 0;
}

/**
 *  @brief      Enable latched interrupts.
 *  Any MPU register will clear the interrupt.
 *  @param[in]  enable  1 to enable, 0 to disable.
 *  @return     0 if successful.
 */
u_int8 mpu_set_int_latched(u_int8 enable)
{
	u_int8 tmp;
	if (st.chip_cfg.latched_int == enable)
		return 0;

	if (enable)
		tmp = BIT_LATCH_EN | BIT_ANY_RD_CLR;
	else
		tmp = 0;
	if (st.chip_cfg.bypass_mode)
		tmp |= BIT_BYPASS_EN;
	if (st.chip_cfg.active_low_int)
		tmp |= BIT_ACTL;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_pin_cfg, tmp)<0)
        return 1;
	
	st.chip_cfg.latched_int = enable;
	return 0;
}

#ifdef MPU6050
static int get_accel_prod_shift(float *st_shift)
{
	u_int8 tmp[4], shift_code[3], ii;

	if (i2c_smbus_read_i2c_block_data(fd_mpu6050, 0x0D, 4, tmp)<0)
		return 0x07;

	shift_code[0] = ((tmp[0] & 0xE0) >> 3) | ((tmp[3] & 0x30) >> 4);
	shift_code[1] = ((tmp[1] & 0xE0) >> 3) | ((tmp[3] & 0x0C) >> 2);
	shift_code[2] = ((tmp[2] & 0xE0) >> 3) | (tmp[3] & 0x03);
	for (ii = 0; ii < 3; ii++)
	{
		if (!shift_code[ii])
		{
			st_shift[ii] = 0.f;
			continue;
		}
		/* Equivalent to..
		 * st_shift[ii] = 0.34f * powf(0.92f/0.34f, (shift_code[ii]-1) / 30.f)
		 */
		st_shift[ii] = 0.34f;
		while (--shift_code[ii])
			st_shift[ii] *= 1.034f;
	}
	return 0;
}

static int accel_self_test(int32_t *bias_regular, int32_t *bias_st)
{
	int jj, result = 0;
	float st_shift[3], st_shift_cust, st_shift_var;

	get_accel_prod_shift(st_shift);
	for(jj = 0; jj < 3; jj++)
	{
		st_shift_cust = labs(bias_regular[jj] - bias_st[jj]) / 65536.f;
		if (st_shift[jj])
		{
			st_shift_var = st_shift_cust / st_shift[jj] - 1.f;
			if (fabs(st_shift_var) > test.max_accel_var)
				result |= 1 << jj;
		}
		else if ((st_shift_cust < test.min_g) ||
						 (st_shift_cust > test.max_g))
			result |= 1 << jj;
	}

	return result;
}

static int gyro_self_test(int32_t *bias_regular, int32_t *bias_st)
{
	int jj, result = 0;
	u_int8 tmp[3];
	float st_shift, st_shift_cust, st_shift_var;

	if (i2c_smbus_read_i2c_block_data(fd_mpu6050, 0x0D, 3, tmp)<0)
		return 0x07;

	tmp[0] &= 0x1F;
	tmp[1] &= 0x1F;
	tmp[2] &= 0x1F;

	for (jj = 0; jj < 3; jj++)
	{
		st_shift_cust = labs(bias_regular[jj] - bias_st[jj]) / 65536.f;
		if (tmp[jj])
		{
			st_shift = 3275.f / test.gyro_sens;
			while (--tmp[jj])
				st_shift *= 1.046f;
			st_shift_var = st_shift_cust / st_shift - 1.f;
			if (fabs(st_shift_var) > test.max_gyro_var)
				result |= 1 << jj;
		}
		else if ((st_shift_cust < test.min_dps) ||
						 (st_shift_cust > test.max_dps))
			result |= 1 << jj;
	}
	return result;
}

#ifdef AK89xx_SECONDARY
static int compass_self_test(void)
{
	u_int8 tmp[6];
	u_int8 tries = 10;
	int result = 0x07;
	int16_t data;

	mpu_set_bypass(1);

	tmp[0] = AKM_POWER_DOWN;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, tmp)<0)
		return 0x07;
	tmp[0] = AKM_BIT_SELF_TEST;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_ASTC, tmp)<0)
		goto AKM_restore;
	tmp[0] = AKM_MODE_SELF_TEST;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, tmp)<0)
		goto AKM_restore;

	do
	{
		delay_ms(10);
		if ((tmp[0]=i2c_smbus_read_byte_data(st.chip_cfg.compass_addr, AKM_REG_ST1))<0)
			goto AKM_restore;
		if (tmp[0] & AKM_DATA_READY)
			break;
	}
	while (tries--);
	if (!(tmp[0] & AKM_DATA_READY))
		goto AKM_restore;

	if (i2c_smbus_read_i2c_block_data(st.chip_cfg.compass_addr, AKM_REG_HXL, 6, tmp)<0)
		goto AKM_restore;

	result = 0;
	data = (int16_t)(tmp[1] << 8) | tmp[0];
	if ((data > 100) || (data < -100))
		result |= 0x01;
	data = (int16_t)(tmp[3] << 8) | tmp[2];
	if ((data > 100) || (data < -100))
		result |= 0x02;
	data = (int16_t)(tmp[5] << 8) | tmp[4];
	if ((data > -300) || (data < -1000))
		result |= 0x04;

AKM_restore:
	tmp[0] = 0 | SUPPORTS_AK89xx_HIGH_SENS;
	i2c_smbus_write_byte(st.chip_cfg.compass_addr, AKM_REG_ASTC, tmp[0]);
	tmp[0] = SUPPORTS_AK89xx_HIGH_SENS;
	i2c_smbus_write_byte(st.chip_cfg.compass_addr, AKM_REG_CNTL, tmp[0]);
	mpu_set_bypass(0);
	return result;
}
#endif
#endif

static int get_st_biases(int32_t *gyro, int32_t *accel, u_int8 hw_test)
{
	u_int8 data[MAX_PACKET_LENGTH];
	u_int8 packet_count, ii;
	u_int16 fifo_count;

	data[0] = 0x01;
	data[1] = 0;
	if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->pwr_mgmt_1, data[0])<0)
        return 1;

	delay_ms(200);
	data[0] = 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data[0])<0)
        return 1;
	
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, data[0])<0)
		return 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, data[0])<0)
		return 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->i2c_mst, data[0])<0)
		return 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data[0])<0)
		return 1;
	data[0] = BIT_FIFO_RST | BIT_DMP_RST;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data[0])<0)
		return 1;
	delay_ms(15);
	data[0] = st.test->reg_lpf;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->lpf, data[0])<0)
		return 1;
	data[0] = st.test->reg_rate_div;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->rate_div, data[0])<0)
		return 1;
	if (hw_test)
		data[0] = st.test->reg_gyro_fsr | 0xE0;
	else
		data[0] = st.test->reg_gyro_fsr;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->gyro_cfg, data[0])<0)
		return 1;

	if (hw_test)
		data[0] = st.test->reg_accel_fsr | 0xE0;
	else
		data[0] = test.reg_accel_fsr;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_cfg, data[0])<0)
		return 1;
	if (hw_test)
		delay_ms(200);

	/* Fill FIFO for test.wait_ms milliseconds. */
	data[0] = BIT_FIFO_EN;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->user_ctrl, data[0])<0)
		return 1;

	data[0] = INV_XYZ_GYRO | INV_XYZ_ACCEL;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, data[0])<0)
		return 1;
	delay_ms(test.wait_ms);
	data[0] = 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->fifo_en, data[0])<0)
		return 1;

	if (i2c_smbus_read_word_data(fd_mpu6050, st.reg->fifo_count_h, data[0])<0)
		return 1;

	fifo_count = (data[0] << 8) | data[1];
	packet_count = fifo_count / MAX_PACKET_LENGTH;
	gyro[0] = gyro[1] = gyro[2] = 0;
	accel[0] = accel[1] = accel[2] = 0;

	for (ii = 0; ii < packet_count; ii++)
	{
		int16_t accel_cur[3], gyro_cur[3];
		if (i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->fifo_r_w, MAX_PACKET_LENGTH, data)<0)
			return 1;
		accel_cur[0] = ((int16_t)data[0] << 8) | data[1];
		accel_cur[1] = ((int16_t)data[2] << 8) | data[3];
		accel_cur[2] = ((int16_t)data[4] << 8) | data[5];
		accel[0] += (int32_t)accel_cur[0];
		accel[1] += (int32_t)accel_cur[1];
		accel[2] += (int32_t)accel_cur[2];
		gyro_cur[0] = (((int16_t)data[6] << 8) | data[7]);
		gyro_cur[1] = (((int16_t)data[8] << 8) | data[9]);
		gyro_cur[2] = (((int16_t)data[10] << 8) | data[11]);
		gyro[0] += (int32_t)gyro_cur[0];
		gyro[1] += (int32_t)gyro_cur[1];
		gyro[2] += (int32_t)gyro_cur[2];
	}
#ifdef EMPL_NO_64BIT
	gyro[0] = (int32_t)(((float)gyro[0]*65536.f) / test.gyro_sens / packet_count);
	gyro[1] = (int32_t)(((float)gyro[1]*65536.f) / test.gyro_sens / packet_count);
	gyro[2] = (int32_t)(((float)gyro[2]*65536.f) / test.gyro_sens / packet_count);
	if (has_accel)
	{
		accel[0] = (int32_t)(((float)accel[0]*65536.f) / test.accel_sens /
												 packet_count);
		accel[1] = (int32_t)(((float)accel[1]*65536.f) / test.accel_sens /
												 packet_count);
		accel[2] = (int32_t)(((float)accel[2]*65536.f) / test.accel_sens /
												 packet_count);
		/* Don't remove gravity! */
		accel[2] -= 65536L;
	}
#else
	gyro[0] = (int32_t)(((int64_t)gyro[0]<<16) / test.gyro_sens / packet_count);
	gyro[1] = (int32_t)(((int64_t)gyro[1]<<16) / test.gyro_sens / packet_count);
	gyro[2] = (int32_t)(((int64_t)gyro[2]<<16) / test.gyro_sens / packet_count);
	accel[0] = (int32_t)(((int64_t)accel[0]<<16) / test.accel_sens /
											 packet_count);
	accel[1] = (int32_t)(((int64_t)accel[1]<<16) / test.accel_sens /
											 packet_count);
	accel[2] = (int32_t)(((int64_t)accel[2]<<16) / test.accel_sens /
											 packet_count);
	/* Don't remove gravity! */
	if (accel[2] > 0L)
		accel[2] -= 65536L;
	else
		accel[2] += 65536L;
#endif

	return 0;
}

/**
 *  @brief      Trigger gyro/accel/compass self-test.
 *  On success/error, the self-test returns a mask representing the sensor(s)
 *  that failed. For each bit, a one (1) represents a "pass" case; conversely,
 *  a zero (0) indicates a failure.
 *
 *  \n The mask is defined as follows:
 *  \n Bit 0:   Gyro.
 *  \n Bit 1:   Accel.
 *  \n Bit 2:   Compass.
 *
 *  \n Currently, the hardware self-test is unsupported for MPU6500. However,
 *  this function can still be used to obtain the accel and gyro biases.
 *
 *  \n This function must be called with the device either face-up or face-down
 *  (z-axis is parallel to gravity).
 *  @param[out] gyro        Gyro biases in q16 format.
 *  @param[out] accel       Accel biases (if applicable) in q16 format.
 *  @return     Result mask (see above).
 */
u_int8 mpu_run_self_test(int32_t *gyro, int32_t *accel)
{
#ifdef MPU6050
	const u_int8 tries = 2;
	int32_t gyro_st[3], accel_st[3];
	u_int8 accel_result, gyro_result;
#ifdef AK89xx_SECONDARY
	u_int8 compass_result;
#endif
	int ii;
#endif
	int result;
	u_int8 accel_fsr, fifo_sensors, sensors_on;
	u_int16 gyro_fsr, sample_rate, lpf;
	u_int8 dmp_was_on;

	if (st.chip_cfg.dmp_on)
	{
		mpu_set_dmp_state(0);
		dmp_was_on = 1;
	}
	else
		dmp_was_on = 0;

	/* Get initial settings. */
	mpu_get_gyro_fsr(&gyro_fsr);
	mpu_get_accel_fsr(&accel_fsr);
	mpu_get_lpf(&lpf);
	mpu_get_sample_rate(&sample_rate);
	sensors_on = st.chip_cfg.sensors;
	mpu_get_fifo_config(&fifo_sensors);

	/* For older chips, the self-test will be different. */
#ifdef MPU6050
	for (ii = 0; ii < tries; ii++)
		if (!get_st_biases(gyro, accel, 0))
			break;
	if (ii == tries)
	{
		/* If we reach this point, we most likely encountered an I2C error.
		 * We'll just report an error for all three sensors.
		 */
		result = 0;
		goto restore;
	}
	for (ii = 0; ii < tries; ii++)
		if (!get_st_biases(gyro_st, accel_st, 1))
			break;
	if (ii == tries)
	{
		/* Again, probably an I2C error. */
		result = 0;
		goto restore;
	}
	accel_result = accel_self_test(accel, accel_st);
	gyro_result = gyro_self_test(gyro, gyro_st);

	result = 0;
	if (!gyro_result)
		result |= 0x01;
	if (!accel_result)
		result |= 0x02;

#ifdef AK89xx_SECONDARY
	compass_result = compass_self_test();
	if (!compass_result)
		result |= 0x04;
#endif
restore:
#elif defined MPU6500
	/* For now, this function will return a "pass" result for all three sensors
	 * for compatibility with current test applications.
	 */
	get_st_biases(gyro, accel, 0);
	result = 0x7;
#endif
	/* Set to invalid values to ensure no I2C writes are skipped. */
	st.chip_cfg.gyro_fsr = 0xFF;
	st.chip_cfg.accel_fsr = 0xFF;
	st.chip_cfg.lpf = 0xFF;
	st.chip_cfg.sample_rate = 0xFFFF;
	st.chip_cfg.sensors = 0xFF;
	st.chip_cfg.fifo_enable = 0xFF;
	st.chip_cfg.clk_src = INV_CLK_PLL;
	mpu_set_gyro_fsr(gyro_fsr);
	mpu_set_accel_fsr(accel_fsr);
	mpu_set_lpf(lpf);
	mpu_set_sample_rate(sample_rate);
	mpu_set_sensors(sensors_on);
	mpu_configure_fifo(fifo_sensors);

	if (dmp_was_on)
		mpu_set_dmp_state(1);

	return result;
}

/**
 *  @brief      Write to the DMP memory.
 *  This function prevents I2C writes past the bank boundaries. The DMP memory
 *  is only accessible when the chip is awake.
 *  @param[in]  mem_addr    Memory location (bank << 8 | start address)
 *  @param[in]  length      Number of bytes to write.
 *  @param[in]  data        Bytes to write to memory.
 *  @return     0 if successful.
 */
u_int8 mpu_write_mem(u_int16 mem_addr, u_int16 length,
											u_int8 *data)
{
	u_int8 tmp[2];

	if (!data)
		return 1;
	if (!st.chip_cfg.sensors)
		return 1;

	tmp[0] = (u_int8)(mem_addr >> 8);
	tmp[1] = (u_int8)(mem_addr & 0xFF);

	/* Check bank boundaries. */
	if (tmp[1] + length > st.hw->bank_size)
		return 1;

	if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->bank_sel, tmp)<0)
		return 1;
	if(i2c_smbus_write_block_data(fd_mpu6050, st.reg->mem_r_w, length, data)<0)
		return 1;
	return 0;
}

/**
 *  @brief      Read from the DMP memory.
 *  This function prevents I2C reads past the bank boundaries. The DMP memory
 *  is only accessible when the chip is awake.
 *  @param[in]  mem_addr    Memory location (bank << 8 | start address)
 *  @param[in]  length      Number of bytes to read.
 *  @param[out] data        Bytes read from memory.
 *  @return     0 if successful.
 */
u_int8 mpu_read_mem(u_int16 mem_addr, u_int16 length,
										 u_int8 *data)
{
	u_int8 tmp[2];

	if (!data)
		return 1;
	if (!st.chip_cfg.sensors)
		return 1;

	tmp[0] = (u_int8)(mem_addr >> 8);
	tmp[1] = (u_int8)(mem_addr & 0xFF);

	/* Check bank boundaries. */
	if (tmp[1] + length > st.hw->bank_size)
		return 1;

	if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->bank_sel, tmp)<0)
		return 1;
	if (i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->mem_r_w, length, data)<0)
		return 1;
	return 0;
}

/**
 *  @brief      Load and verify DMP image.
 *  @param[in]  length      Length of DMP image.
 *  @param[in]  firmware    DMP code.
 *  @param[in]  start_addr  Starting address of DMP code memory.
 *  @param[in]  sample_rate Fixed sampling rate used when DMP is enabled.
 *  @return     0 if successful.
 */
u_int8 mpu_load_firmware(u_int16 length, const u_int8 *firmware,
						 u_int16 start_addr, u_int16 sample_rate)
{
	u_int16 ii;
	u_int16 this_write;
	u_int8 *pgm_buf;
	u_int8 jj;
	/* Must divide evenly into st.hw->bank_size to avoid bank crossings. */
#define LOAD_CHUNK  (16)
	u_int8 cur[LOAD_CHUNK], tmp[2];
	pgm_buf = (u_int8 *)malloc(LOAD_CHUNK);

	if (st.chip_cfg.dmp_loaded)
		/* DMP should only be loaded once. */
		return 1;

	if (!firmware)
		return 1;
	for (ii = 0; ii < length; ii += this_write)
	{
		this_write = min(LOAD_CHUNK, length - ii);		
		for (jj = 0; jj < LOAD_CHUNK; jj++) pgm_buf[jj] = *(firmware + ii + jj);
		if (mpu_write_mem(ii, this_write, pgm_buf))
			return 1;
		if (mpu_read_mem(ii, this_write, cur))
			return 1;
		if (memcmp(pgm_buf, cur, this_write))
			return 2;
	}

	/* Set program start address. */
	tmp[0] = start_addr >> 8;
	tmp[1] = start_addr & 0xFF;
	if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->prgm_start_h, tmp)<0)
		return 1;

	st.chip_cfg.dmp_loaded = 1;
	st.chip_cfg.dmp_sample_rate = sample_rate;
	return 0;
}

/**
 *  @brief      Enable/disable DMP support.
 *  @param[in]  enable  1 to turn on the DMP.
 *  @return     0 if successful.
 */
u_int8 mpu_set_dmp_state(u_int8 enable)
{
	u_int8 tmp;
	if (st.chip_cfg.dmp_on == enable)
		return 0;

	if (enable)
	{
		if (!st.chip_cfg.dmp_loaded)
			return 1;
		/* Disable data ready interrupt. */
		set_int_enable(0);
		/* Disable bypass mode. */
		mpu_set_bypass(0);
		/* Keep constant sample rate, FIFO rate controlled by DMP. */
		mpu_set_sample_rate(st.chip_cfg.dmp_sample_rate);
		/* Remove FIFO elements. */
		tmp = 0;
		i2c_smbus_write_byte_data(fd_mpu6050, 0x23, tmp);
		st.chip_cfg.dmp_on = 1;
		/* Enable DMP interrupt. */
		set_int_enable(1);
		mpu_reset_fifo();
	}
	else
	{
		/* Disable DMP interrupt. */
		set_int_enable(0);
		/* Restore FIFO settings. */
		tmp = st.chip_cfg.fifo_enable;
		i2c_smbus_write_byte_data(fd_mpu6050, 0x23, tmp);
		st.chip_cfg.dmp_on = 0;
		mpu_reset_fifo();
	}
	return 0;
}

/**
 *  @brief      Get DMP state.
 *  @param[out] enabled 1 if enabled.
 *  @return     0 if successful.
 */
u_int8 mpu_get_dmp_state(u_int8 *enabled)
{
	enabled[0] = st.chip_cfg.dmp_on;
	return 0;
}


/* This initialization is similar to the one in ak8975.c. */
static int setup_compass(void)
{
#ifdef AK89xx_SECONDARY
	u_int8 data[4], akm_addr;
#ifdef MPU_DEBUG
	printf("Initializing compass...\r\n");
#endif
	mpu_set_bypass(1);

	/* Find compass. Possible addresses range from 0x0C to 0x0F. */
	for (akm_addr = 0x0C; akm_addr <= 0x0F; akm_addr++)
	{
		int result;
		result = i2c_smbus_read_byte_data(akm_addr, AKM_REG_WHOAMI, 1, data);
		if (!result && (data[0] == AKM_WHOAMI))
			break;
	}

	if (akm_addr > 0x0F)
	{
		/* TODO: Handle this case in all compass-related functions. */
#ifdef MPU_DEBUG
		printf("Compass not found.\r\n");
#endif
		return 1;
	}

	st.chip_cfg.compass_addr = akm_addr;

	data[0] = AKM_POWER_DOWN;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, data)<0)
		return 1;
	delay_ms(1);

	data[0] = AKM_FUSE_ROM_ACCESS;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, data)<0)
		return 1;
	delay_ms(1);

	/* Get sensitivity adjustment data from fuse ROM. */
	if (i2c_smbus_read_i2c_block_data(st.chip_cfg.compass_addr, AKM_REG_ASAX, 3, data)<0)
		return 1;
	st.chip_cfg.mag_sens_adj[0] = (int32_t)data[0] + 128;
	st.chip_cfg.mag_sens_adj[1] = (int32_t)data[1] + 128;
	st.chip_cfg.mag_sens_adj[2] = (int32_t)data[2] + 128;

	data[0] = AKM_POWER_DOWN;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, data)<0)
		return 1;
	delay_ms(1);

	mpu_set_bypass(0);

	/* Set up master mode, master clock, and ES bit. */
	data[0] = 0x40;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->i2c_mst, data)<0)
		return 1;

	/* Slave 0 reads from AKM data registers. */
	data[0] = BIT_i2c_smbus_read_byte_data | st.chip_cfg.compass_addr;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s0_addr, data)<0)
		return 1;

	/* Compass reads start at this register. */
	data[0] = AKM_REG_ST1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s0_reg, data)<0)
		return 1;

	/* Enable slave 0, 8-byte reads. */
	data[0] = BIT_SLAVE_EN | 8;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s0_ctrl, data)<0)
		return 1;

	/* Slave 1 changes AKM measurement mode. */
	data[0] = st.chip_cfg.compass_addr;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s1_addr, data)<0)
		return 1;

	/* AKM measurement mode register. */
	data[0] = AKM_REG_CNTL;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s1_reg, data)<0)
		return 1;

	/* Enable slave 1, 1-byte writes. */
	data[0] = BIT_SLAVE_EN | 1;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s1_ctrl, data)<0)
		return 1;

	/* Set slave 1 data. */
	data[0] = AKM_SINGLE_MEASUREMENT;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->s1_do, data)<0)
		return 1;

	/* Trigger slave 0 and slave 1 actions at each sample. */
	data[0] = 0x03;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->i2c_delay_ctrl, data)<0)
		return 1;

#ifdef MPU9150
	/* For the MPU9150, the auxiliary I2C bus needs to be set to VDD. */
#ifdef MPU_DEBUG
	printf("Initializing compass at IMU...\r\n");
#endif
	data[0] = BIT_I2C_MST_VDDIO;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->yg_offs_tc, data)<0)
		return 1;
#endif

	return 0;
#else
	return 1;
#endif
}

/**
 *  @brief      Read raw compass data.
 *  @param[out] data        Raw data in hardware units.
 *  @return     0 if successful.
 */
u_int8 mpu_get_compass_reg(int16_t *data)
{
#ifdef AK89xx_SECONDARY
	u_int8 tmp[9];

	if (!(st.chip_cfg.sensors & INV_XYZ_COMPASS))
		return 1;

#ifdef AK89xx_BYPASS
	if (i2c_smbus_read_i2c_block_data(st.chip_cfg.compass_addr, AKM_REG_ST1, 8, tmp)<0)
		return 1;
	tmp[8] = AKM_SINGLE_MEASUREMENT;
	if(i2c_smbus_write_byte_data(st.chip_cfg.compass_addr, AKM_REG_CNTL, tmp+8)<0)
		return 1;
#else
	if (i2c_smbus_read_i2c_block_data(fd_mpu6050, st.reg->raw_compass, 8, tmp)<0)
		return 1;
#endif

#ifdef AK8975_SECONDARY
	/* AK8975 doesn't have the overrun error bit. */
	if (!(tmp[0] & AKM_DATA_READY))
		return 2;
	if ((tmp[7] & AKM_OVERFLOW) || (tmp[7] & AKM_DATA_ERROR))
		return 3;
#elif defined AK8963_SECONDARY
	/* AK8963 doesn't have the data read error bit. */
	if (!(tmp[0] & AKM_DATA_READY) || (tmp[0] & AKM_DATA_OVERRUN))
		return 2;
	if (tmp[7] & AKM_OVERFLOW)
		return 3;
#endif
	data[0] = (tmp[2] << 8) | tmp[1];
	data[1] = (tmp[4] << 8) | tmp[3];
	data[2] = (tmp[6] << 8) | tmp[5];

	data[0] = ((int32_t)data[0] * st.chip_cfg.mag_sens_adj[0]) >> 8;
	data[1] = ((int32_t)data[1] * st.chip_cfg.mag_sens_adj[1]) >> 8;
	data[2] = ((int32_t)data[2] * st.chip_cfg.mag_sens_adj[2]) >> 8;

	return 0;
#else
	return 1;
#endif
}

/**
 *  @brief      Get the compass full-scale range.
 *  @param[out] fsr Current full-scale range.
 *  @return     0 if successful.
 */
u_int8 mpu_get_compass_fsr(u_int16 *fsr)
{
#ifdef AK89xx_SECONDARY
	fsr[0] = st.hw->compass_fsr;
	return 0;
#else
	return 1;
#endif
}

/**
 *  @brief      Enters LP accel motion interrupt mode.
 *  The behavior of this feature is very different between the MPU6050 and the
 *  MPU6500. Each chip's version of this feature is explained below.
 *
 *  \n MPU6050:
 *  \n When this mode is first enabled, the hardware captures a single accel
 *  sample, and subsequent samples are compared with this one to determine if
 *  the device is in motion. Therefore, whenever this "locked" sample needs to
 *  be changed, this function must be called again.
 *
 *  \n The hardware motion threshold can be between 32mg and 8160mg in 32mg
 *  increments.
 *
 *  \n Low-power accel mode supports the following frequencies:
 *  \n 1.25Hz, 5Hz, 20Hz, 40Hz
 *
 *  \n MPU6500:
 *  \n Unlike the MPU6050 version, the hardware does not "lock in" a reference
 *  sample. The hardware monitors the accel data and detects any large change
 *  over a int16_t period of time.
 *
 *  \n The hardware motion threshold can be between 4mg and 1020mg in 4mg
 *  increments.
 *
 *  \n MPU6500 Low-power accel mode supports the following frequencies:
 *  \n 1.25Hz, 2.5Hz, 5Hz, 10Hz, 20Hz, 40Hz, 80Hz, 160Hz, 320Hz, 640Hz
 *
 *  \n\n NOTES:
 *  \n The driver will round down @e thresh to the nearest supported value if
 *  an unsupported threshold is selected.
 *  \n To select a fractional wake-up frequency, round down the value passed to
 *  @e lpa_freq.
 *  \n The MPU6500 does not support a delay parameter. If this function is used
 *  for the MPU6500, the value passed to @e time will be ignored.
 *  \n To disable this mode, set @e lpa_freq to zero. The driver will restore
 *  the previous configuration.
 *
 *  @param[in]  thresh      Motion threshold in mg.
 *  @param[in]  time        Duration in milliseconds that the accel data must
 *                          exceed @e thresh before motion is reported.
 *  @param[in]  lpa_freq    Minimum sampling rate, or zero to disable.
 *  @return     0 if successful.
 */
u_int8 mpu_lp_motion_interrupt(u_int16 thresh, u_int8 time, u_int8 lpa_freq)
{
	u_int8 data[3];

	if (lpa_freq)
	{
		u_int8 thresh_hw;

#ifdef MPU6050
		/* TODO: Make these const/#defines. */
		/* 1LSb = 32mg. */
		if (thresh > 8160)
			thresh_hw = 255;
		else if (thresh < 32)
			thresh_hw = 1;
		else
			thresh_hw = thresh >> 5;
#elif defined MPU6500
		/* 1LSb = 4mg. */
		if (thresh > 1020)
			thresh_hw = 255;
		else if (thresh < 4)
			thresh_hw = 1;
		else
			thresh_hw = thresh >> 2;
#endif

		if (!time)
			/* Minimum duration must be 1ms. */
			time = 1;

#ifdef MPU6050
		if (lpa_freq > 40)
#elif defined MPU6500
		if (lpa_freq > 640)
#endif
			/* At this point, the chip has not been re-configured, so the
			 * function can safely exit.
			 */
			return 1;

		if (!st.chip_cfg.int_motion_only)
		{
			/* Store current settings for later. */
			if (st.chip_cfg.dmp_on)
			{
				mpu_set_dmp_state(0);
				st.chip_cfg.cache.dmp_on = 1;
			}
			else
				st.chip_cfg.cache.dmp_on = 0;
			mpu_get_gyro_fsr(&st.chip_cfg.cache.gyro_fsr);
			mpu_get_accel_fsr(&st.chip_cfg.cache.accel_fsr);
			mpu_get_lpf(&st.chip_cfg.cache.lpf);
			mpu_get_sample_rate(&st.chip_cfg.cache.sample_rate);
			st.chip_cfg.cache.sensors_on = st.chip_cfg.sensors;
			mpu_get_fifo_config(&st.chip_cfg.cache.fifo_sensors);
		}

#ifdef MPU6050
		/* Disable hardware interrupts for now. */
		set_int_enable(0);

		/* Enter full-power accel-only mode. */
		mpu_lp_accel_mode(0);

		/* Override current LPF (and HPF) settings to obtain a valid accel
		 * reading.
		 */
		data[0] = INV_FILTER_256HZ_NOLPF2;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->lpf, data[0])<0)
			return 1;

		/* NOTE: Digital high pass filter should be configured here. Since this
		 * driver doesn't modify those bits anywhere, they should already be
		 * cleared by default.
		 */

		/* Configure the device to send motion interrupts. */
		/* Enable motion interrupt. */
		data[0] = BIT_MOT_INT_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data[0])<0)
			goto lp_int_restore;

		/* Set motion interrupt parameters. */
		data[0] = thresh_hw;
		data[1] = time;
		if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->motion_thr, (data[0]<<8)|data[1])<0)
			goto lp_int_restore;

		/* Force hardware to "lock" current accel sample. */
		delay_ms(5);
		data[0] = (st.chip_cfg.accel_fsr << 3) | BITS_HPF;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_cfg, data[0])<0)
			goto lp_int_restore;

		/* Set up LP accel mode. */
		data[0] = BIT_LPA_CYCLE;
		if (lpa_freq == 1)
			data[1] = INV_LPA_1_25HZ;
		else if (lpa_freq <= 5)
			data[1] = INV_LPA_5HZ;
		else if (lpa_freq <= 20)
			data[1] = INV_LPA_20HZ;
		else
			data[1] = INV_LPA_40HZ;
		data[1] = (data[1] << 6) | BIT_STBY_XYZG;
		if(i2c_smbus_write_word_data(fd_mpu6050, st.reg->pwr_mgmt_1, (data[0]<<8)|data[1])<0)
			goto lp_int_restore;

		st.chip_cfg.int_motion_only = 1;
		return 0;
#elif defined MPU6500
		/* Disable hardware interrupts. */
		set_int_enable(0);

		/* Enter full-power accel-only mode, no FIFO/DMP. */
		data[0] = 0;
		data[1] = 0;
		data[2] = BIT_STBY_XYZG;
		if(i2c_smbus_write_block_data(fd_mpu6050, st.reg->user_ctrl, 3, data)<0)
			goto lp_int_restore;

		/* Set motion threshold. */
		data[0] = thresh_hw;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->motion_thr, data)<0)
			goto lp_int_restore;

		/* Set wake frequency. */
		if (lpa_freq == 1)
			data[0] = INV_LPA_1_25HZ;
		else if (lpa_freq == 2)
			data[0] = INV_LPA_2_5HZ;
		else if (lpa_freq <= 5)
			data[0] = INV_LPA_5HZ;
		else if (lpa_freq <= 10)
			data[0] = INV_LPA_10HZ;
		else if (lpa_freq <= 20)
			data[0] = INV_LPA_20HZ;
		else if (lpa_freq <= 40)
			data[0] = INV_LPA_40HZ;
		else if (lpa_freq <= 80)
			data[0] = INV_LPA_80HZ;
		else if (lpa_freq <= 160)
			data[0] = INV_LPA_160HZ;
		else if (lpa_freq <= 320)
			data[0] = INV_LPA_320HZ;
		else
			data[0] = INV_LPA_640HZ;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->lp_accel_odr, data)<0)
			goto lp_int_restore;

		/* Enable motion interrupt (MPU6500 version). */
		data[0] = BITS_WOM_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_intel, data)<0)
			goto lp_int_restore;

		/* Enable cycle mode. */
		data[0] = BIT_LPA_CYCLE;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->pwr_mgmt_1, data)<0)
			goto lp_int_restore;

		/* Enable interrupt. */
		data[0] = BIT_MOT_INT_EN;
		if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->int_enable, data)<0)
			goto lp_int_restore;

		st.chip_cfg.int_motion_only = 1;
		return 0;
#endif
	}
	else
	{
		/* Don't "restore" the previous state if no state has been saved. */
		int ii;
		char *cache_ptr = (char*)&st.chip_cfg.cache;
		for (ii = 0; ii < sizeof(st.chip_cfg.cache); ii++)
		{
			if (cache_ptr[ii] != 0)
				goto lp_int_restore;
		}
		/* If we reach this point, motion interrupt mode hasn't been used yet. */
		return 1;
	}
lp_int_restore:
	/* Set to invalid values to ensure no I2C writes are skipped. */
	st.chip_cfg.gyro_fsr = 0xFF;
	st.chip_cfg.accel_fsr = 0xFF;
	st.chip_cfg.lpf = 0xFF;
	st.chip_cfg.sample_rate = 0xFFFF;
	st.chip_cfg.sensors = 0xFF;
	st.chip_cfg.fifo_enable = 0xFF;
	st.chip_cfg.clk_src = INV_CLK_PLL;
	mpu_set_sensors(st.chip_cfg.cache.sensors_on);
	mpu_set_gyro_fsr(st.chip_cfg.cache.gyro_fsr);
	mpu_set_accel_fsr(st.chip_cfg.cache.accel_fsr);
	mpu_set_lpf(st.chip_cfg.cache.lpf);
	mpu_set_sample_rate(st.chip_cfg.cache.sample_rate);
	mpu_configure_fifo(st.chip_cfg.cache.fifo_sensors);

	if (st.chip_cfg.cache.dmp_on)
		mpu_set_dmp_state(1);

#ifdef MPU6500
	/* Disable motion interrupt (MPU6500 version). */
	data[0] = 0;
	if(i2c_smbus_write_byte_data(fd_mpu6050, st.reg->accel_intel, data)<0)
		goto lp_int_restore;
#endif

	st.chip_cfg.int_motion_only = 0;
	return 0;
}

/**
 *  @}
 */

// packet structure for InvenSense teapot demo
int8_t teapotPacket[24] = {'$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\r', '\n'};

uint16_t dmp_features;

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from thei
 * driver(s).
 * TODO: The following matrices refer to the configuration on an internal test
 * board at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static int8_t gyro_orientation[9] = {1, 0, 0,
				     0, 1, 0,
				     0, 0, 1};

void mpu6050_init() 
{
    fd_mpu6050 = i2c_open(1, MPU6050_DEFAULT_ADDRESS);

    set_clock_source(MPU6050_CLOCK_PLL_XGYRO);
    set_full_scale_gyro_range(MPU6050_GYRO_FS_500);
    set_full_scale_accel_range(MPU6050_ACCEL_FS_2);
    set_sleep_enabled(0);

	if(fd_mpu6050 > 0)
		i2c_close(fd_mpu6050);
}

int gyro_int_fd;
void gyro_regist()
{
    regist_gpio(gyro_pin[0][0], gyro_pin[0][1], DIR_IN);

    gyro_int_fd = open("/sys/class/gpio/gpio66/edge", O_RDWR);
    write(gyro_int_fd, "rising", sizeof("rising"));
    close(gyro_int_fd);

    gyro_int_fd = open("/sys/class/gpio/gpio66/direction", O_RDWR);
    write(gyro_int_fd, "in", sizeof("in"));
    close(gyro_int_fd);

}

void mpu_enable_intterupt()
{
}

void *mpu6050_detect_with_dmp(void *data)
{
	int rc;
	uint8_t accel_fsr;
	uint16_t gyro_rate, gyro_fsr;
int cnt=0;

	int16_t gyro[3], accel[3], sensors;
	int32_t quat[4];
	int16_t compass[3];
	uint8_t more;
	
	char temp_out[4];
	int32_t temp_in;
    int i, len;
	char buf[1024];
    struct pollfd fdset;

	/* Initializing MPU */
	rc = mpu_init(NULL);
#ifdef MPU_DEBUG
	printf("Result of mpu_init %d\r\n", rc);
#endif
    fd_mpu6050 = i2c_open(1, MPU6050_DEFAULT_ADDRESS);

	/* Get/set hardware configuration. Start gyro. */
	/* Wake up all sensors. */
	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

	/* Push both gyro and accel data into the FIFO. */
	mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_set_sample_rate(DEFAULT_MPU_HZ);

	/* Read back configuration in case it was set improperly. */
	mpu_get_sample_rate(&gyro_rate);
	mpu_get_gyro_fsr(&gyro_fsr);
	mpu_get_accel_fsr(&accel_fsr);

	/* Initialize DMP. */
	rc = dmp_load_motion_driver_firmware();
#ifdef MPU_DEBUG
	printf("Result of load firmware %d\r\n", rc);
#endif
	dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
	dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL;
	dmp_enable_feature(dmp_features);
	dmp_set_fifo_rate(DEFAULT_MPU_HZ);

	/* Set hardware to interrupt periodically. */
	dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);

	if(fd_mpu6050 > 0)
		i2c_close(fd_mpu6050);

	delay_ms(500);

    gyro_int_fd = open("/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK);
    if(gyro_int_fd < 0)
    {
        return;
    }

	for(;;)
	{
		/* Enable the DMP */
		mpu_set_dmp_state(1);

        memset((void*)&fdset, 0, sizeof(fdset));

        fdset.fd = gyro_int_fd;
        fdset.events = POLLPRI;
        rc = poll(&fdset, 1, -1);

        if (rc < 0) {
            printf("\npoll() failed!\n");
            close(gyro_int_fd);
            return NULL;
        }

        if (rc == 0) {
            printf(".");
        }

        if (fdset.revents & POLLPRI) {
        	len = read(fdset.fd, buf, sizeof(buf));

			pthread_mutex_lock(&mutex_i2c);
		    fd_mpu6050 = i2c_open(1, MPU6050_DEFAULT_ADDRESS);
			dmp_read_fifo(gyro, accel, quat, &sensors, &more);
			// Read temperature data in q16 format
			mpu_get_temperature(&temp_in);

			// Read compass data
			mpu_get_compass_reg(compass);

			if(fd_mpu6050 > 0)
				i2c_close(fd_mpu6050);

			pthread_mutex_unlock(&mutex_i2c);

        	printf("\npoll() GPIO %d interrupt occurred, cnt=%d, sensor=%d\n", 66, cnt++, sensors);

			if (sensors & INV_XYZ_GYRO) {
				// Send data to the client application in quat packet type format
				teapotPacket[2] = (int8_t)(quat[0] >> 24);
				teapotPacket[3] = (int8_t)(quat[0] >> 16);
				teapotPacket[4] = (int8_t)(quat[0] >> 8);
				teapotPacket[5] = (int8_t)quat[0];
				teapotPacket[6] = (int8_t)(quat[1] >> 24);
				teapotPacket[7] = (int8_t)(quat[1] >> 16);
				teapotPacket[8] = (int8_t)(quat[1] >> 8);
				teapotPacket[9] = (int8_t)quat[1];
				teapotPacket[10] = (int8_t)(quat[2] >> 24);
				teapotPacket[11] = (int8_t)(quat[2] >> 16);
				teapotPacket[12] = (int8_t)(quat[2] >> 8);
				teapotPacket[13] = (int8_t)quat[2];
				teapotPacket[14] = (int8_t)(quat[3] >> 24);
				teapotPacket[15] = (int8_t)(quat[3] >> 16);
				teapotPacket[16] = (int8_t)(quat[3] >> 8);
				teapotPacket[17] = (int8_t)quat[3];

				// Convert temperature to ASCII format
//				dtostrf((float)(temp_in/pow(2, 16)),4,1,temp_out);

				teapotPacket[18] = temp_out[0];
				teapotPacket[19] = temp_out[1];
				teapotPacket[20] = temp_out[2];
				teapotPacket[21] = temp_out[3];

				for (i = 0; i < 24; ++i)
				{
					printf("%c", teapotPacket[i]);
				}

				printf("\ngyro: %d\t%d\t%d\t\r\n", gyro[0], gyro[1], gyro[2]);
				printf("accel: %d\t%d\t%d\t\r\n", accel[0], accel[1], accel[2]);
//				printf("comp: %d\t%d\t%d\t\r\n", compass[0], compass[2], compass[2]);
				printf("quat: %ld\t%ld\t%ld\t%ld\t\r\n", quat[0], quat[1], quat[2], quat[3]);

				// for print value in float format you can use floating point printf version (see
				// Makefile)
				printf("%.1f\r\n", temp_in/(pow(2, 16)));
			}
		}

        delay_ms(100);

	}
    close(gyro_int_fd);

    return NULL;
}

void *mpu6050_detect(void *data)
{
	/* Initializing MPU */
	mpu6050_init() ;

	for(;;)
	{
        delay_ms(100);
	}

    return NULL;
}


