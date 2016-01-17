#include "gpio.h"

#ifdef _cplusplus
extern "C" {
#endif

#ifndef GPIO_USING_DTS

const char *gpio_name[92][8] = {
    {"GND",             "GND",                  "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"GND",             "GND",                  "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"gpmc_ad6",        "mmc1_dat6",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_6"},
    {"gpmc_ad7",        "mmc1_dat7",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_7"},
    {"gpmc_ad2",        "mmc1_dat2",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_2"},
    {"gpmc_ad3",        "mmc1_dat3",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_3"},
    {"gpmc_advn_ale",   "NULL",                    "timer4",            "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio2_2"},
    {"gpmc_oen_ren",    "NULL",                    "timer7",            "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio2_3"},
    {"gpmc_be0n_cle",   "NULL",                    "timer5",            "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio2_5"},
    {"gpmc_wen",        "NULL",                    "timer6",            "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio2_4"},
    {"gpmc_ad13",       "lcd_data18",            "mmc1_dat5",        "mmc2_dat1",        "eQEP2B_in",            "NULL",                    "pr1_pru0_pru_r30_15",    "gpio1_13"},
    {"gpmc_ad12",       "lcd_data19",            "mmc1_dat4",        "mmc2_dat0",        "Eqep2a_in",            "NULL",                    "pr1_pru0_pru_r30_14",    "gpio1_12"},
    {"gpmc_ad9",        "lcd_data22",            "mmc1_dat1",        "mmc2_dat5",        "ehrpwm2B",                "NULL",                    "NULL",                    "gpio0_23"},
    {"gpmc_ad10",       "lcd_data21",            "mmc1_dat2",        "mmc2_dat6",        "ehrpwm2_tripzone_in",  "NULL",                    "NULL",                    "gpio0_26"},
    {"gpmc_ad15",       "lcd_data16",            "mmc1_dat7",        "mmc2_dat3",        "eQEP2_strobe",            "NULL",                    "pr1_pru0_pru_r31_15",    "gpio1_15"},
    {"gpmc_ad14",       "lcd_data17",            "mmc1_dat6",        "mmc2_dat2",        "eQEP2_index",            "NULL",                    "pr1_pru0_pru_r31_14",    "gpio1_14"},
    {"gpmc_ad11",       "lcd_data20",            "mmc1_dat3",        "mmc2_dat7",        "ehrpwm0_synco",        "NULL",                    "NULL",                    "gpio0_27"},
    {"gpmc_clk_mux0",   "lcd_memory_clk",        "gpmc_wait1",       "mmc2_clk",            "NULL",
 "NULL",                    "mcasp0_fsr",            "gpio2_1"},
    {"gpmc_ad8",        "lcd_data23",            "mmc1_dat0",        "mmc2_dat4",        "ehrpwm2A",                "NULL",                    "NULL",                    "gpio0_22"},
    {"gpmc_csn2",       "gpmc_be1n",            "mmc1_cmd",            "NULL",                "NULL",                    "pr1_pru1_pru_r30_13",  "pr1_pru1_pru_r31_13",    "gpio1_31"},
    {"gpmc_csn1",       "gpmc_clk",                "mmc1_clk",            "NULL",                "NULL",                    "pr1_pru1_pru_r30_12",  "pr1_pru1_pru_r31_12",    "gpio1_30"},
    {"gpmc_ad5",        "mmc1_dat5",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_5"},
    {"gpmc_ad4",        "mmc1_dat4",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_4"},
    {"gpmc_ad1",        "mmc1_dat1",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_1"},
    {"gpmc_ad0",        "mmc1_dat0",            "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_0"},
    {"gpmc_csn0",       "NULL",                    "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio1_29"},
    {"lcd_vsync",       "gpmc_a8",                "NULL",                "NULL",                "NULL",                    "pr1_pru1_pru_r30_8",    "pr1_pru1_pru_r31_8",    "gpio2_22"},
    {"lcd_pclk",        "gpmc_a10",                "NULL",                "NULL",                "NULL",                    "pr1_pru1_pru_r30_10",  "pr1_pru1_pru_r31_10",    "gpio2_24"},
    {"lcd_hsync",       "gpmc_a9",                "NULL",                "NULL",                "NULL",                    "pr1_pru1_pru_r30_9",    "pr1_pru1_pru_r31_9",    "gpio2_23"},
    {"lcd_ac_bias_en",  "gpmc_a11",                "NULL",                "NULL",                "NULL",                    "NULL",                    "NULL",                    "gpio2_25"},
    {"lcd_data14",      "gpmc_a18",                "eQEP1_index",      "mcasp0_axr1",      "uart5_rxd",            "NULL",                    "uart5_ctsn",            "gpio0_10"},
    {"lcd_data15",      "gpmc_a19",                "eQEP1_strobe",     "mcasp0_ahclkx",    "mcasp0_axr3",            "NULL",                    "uart5_rtsn",            "gpio0_11"},
    {"lcd_data13",      "gpmc_a17",                "eQEP1B_in",        "mcasp0_fsr",       "mcasp0_axr3",            "NULL",                    "uart4_rtsn",            "gpio0_9"},
    {"lcd_data11",      "gpmc_a15",                "ehrpwm1B",            "mcasp0_ahclkr",    "mcasp0_axr2",            "NULL",                    "uart3_rtsn",            "gpio2_17"},
    {"lcd_data12",      "gpmc_a16",                "eQEP1A_in",        "mcasp0_aclkr",     "mcasp0_axr2",            "NULL",                    "uart4_ctsn",            "gpio0_8"},
    {"lcd_data10",      "gpmc_a14",                "ehrpwm1A",            "mcasp0_axr0",      "NULL",                    "NULL",                    "uart3_ctsn",            "gpio2_16"},
    {"lcd_data8",       "gpmc_a12",                "ehrpwm1_tripzone_in","mcasp0_aclkx",   "uart5_txd",            "NULL",                    "uart2_ctsn",            "gpio2_14"},
    {"lcd_data9",       "gpmc_a13",                "ehrpwm0_synco",    "mcasp0_fsx",       "uart5_rxd",            "NULL",                    "uart2_rtsn",            "gpio2_15"},
    {"lcd_data6",       "gpmc_a6",                "NULL",                "eQEP2_index",      "NULL",                    "pr1_pru1_pru_r30_6",                    "pr1_pru1_pru_r31_6",            "gpio2_12"},
    {"lcd_data7",       "gpmc_a7",                "NULL",                "eQEP2_strobe",     "pr1_edio_data_out7",   "pr1_pru1_pru_r30_7",                    "pr1_pru1_pru_r31_7",            "gpio2_13"},
    {"lcd_data4",       "gpmc_a4",                "NULL",                "eQEP2A_in",        "NULL",                    "pr1_pru1_pru_r30_4",                    "pr1_pru1_pru_r31_4",            "gpio2_10"},
    {"lcd_data5",       "gpmc_a5",                "NULL",                "eQEP2B_in",        "NULL",                    "pr1_pru1_pru_r30_5",                    "pr1_pru1_pru_r31_5",            "gpio2_11"},
    {"lcd_data2",       "gpmc_a2",                "NULL",                "ehrpwm2_tripzone_in","NULL",                "pr1_pru1_pru_r30_2",                    "pr1_pru1_pru_r31_2",            "gpio2_8"},
    {"lcd_data3",       "gpmc_a3",                "NULL",                "ehrpwm0_synco",    "NULL",                    "pr1_pru1_pru_r30_3",                    "pr1_pru1_pru_r31_3",            "gpio2_9"},
    {"lcd_data0",       "gpmc_a0",                "NULL",                "ehrpwm2A",         "NULL",                    "pr1_pru1_pru_r30_0",                    "pr1_pru1_pru_r31_0",            "gpio2_6"},
    {"lcd_data1",       "gpmc_a1",                "NULL",                "ehrpwm2B",         "NULL",                    "pr1_pru1_pru_r30_1",                    "pr1_pru1_pru_r31_1",            "gpio2_7"},
    {"GND",             "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"GND",             "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"DC_3.3V",         "DC_3.3V",                "DC_3.3V",            "DC_3.3V",            "DC_3.3V",                "DC_3.3V",                "DC_3.3V",                "DC_3.3V"},
    {"DC_3.3V",         "DC_3.3V",                "DC_3.3V",            "DC_3.3V",            "DC_3.3V",                "DC_3.3V",                "DC_3.3V",                "DC_3.3V"},
    {"VDD_5V",          "VDD_5V",                "VDD_5V",            "VDD_5V",            "VDD_5V",                "VDD_5V",                "VDD_5V",                "VDD_5V"},
    {"VDD_5V",          "VDD_5V",                "VDD_5V",            "VDD_5V",            "VDD_5V",                "VDD_5V",                "VDD_5V",                "VDD_5V"},
    {"SYS_5V",          "SYS_5V",                "SYS_5V",            "SYS_5V",            "SYS_5V",                "SYS_5V",                "SYS_5V",                "SYS_5V"},
    {"SYS_5V",          "SYS_5V",                "SYS_5V",            "SYS_5V",            "SYS_5V",                "SYS_5V",                "SYS_5V",                "SYS_5V"},
    {"PWR_BUT",         "PWR_BUT",                "PWR_BUT",            "PWR_BUT",            "PWR_BUT",                "PWR_BUT",                "PWR_BUT",                "PWR_BUT"},
    {"SYS_RESETn",      "SYS_RESETn",            "SYS_RESETn",        "SYS_RESETn",        "SYS_RESETn",            "SYS_RESETn",            "SYS_RESETn",            "SYS_RESETn"},
    {"gpmc_wait0",      "mii2_crs",             "gpmc_csn4",        "rmii2_crs_dv",     "mmc1_sdc",             "NULL",                 "uart4_rxd_mux2",       "gpio0_30"},
    {"gpmc_be1n",       "mii2_col",             "gpmc_csn6",        "rmii2_dat3",       "gpmc_dir",             "NULL",                 "mcasp0_aclkr_mux3",    "gpio1_28"},
    {"gpmc_wpn",        "mii2_rxerr",           "gpmc_csn5",        "rmii2_rxer",       "mmc2_sdcd",            "NULL",                 "uart4_txd_mux2",       "gpio0_31"},
    {"gpmc_a2",         "mii2_txd3",            "rgmii2_td3",       "mmc2_dat1",        "gpmc_a18",             "NULL",                 "ehrpwm1A_mux1",        "gpio1_18"},
    {"gpmc_a0",         "gmii2_txen",           "rmii2_tctl",       "mii2_txen",        "gpmc_a16",             "NULL",                 "ehrpwm1_tripzone_input","gpio1_16"},
    {"gpmc_a3",         "mii2_txd2",            "rgmii2_td2",       "mmc2_dat2",        "gpmc_a19",             "NULL",                 "ehrpwm1B_mux1",        "gpio1_19"},
    {"spi0_cs0",        "mmc2_sdwp",            "I2C1_SCL",         "ehrpwm0_synci",    "pr1_uart0_txd",        "NULL",                 "NULL",                 "gpio0_5"},
    {"spi0_d1",         "mmc1_sdwp",            "I2C1_SDA",         "ehrpwm0_tripzone", "pr1_uart0_rxd",        "NULL",                 "NULL",                 "gpio0_4"},
    {"uart1_rtsn",      "timer5",               "dcan0_rx",         "I2C2_SCL",         "spi1_cs1",             "pr1_uart0_rts_n",      "NULL",                 "gpio0_13"},
    {"uart1_ctsn",      "timer6",               "dcan0_tx",         "I2C2_SDA",         "spi1_cs0",             "pr1_uart0_cts_n",      "NULL",                 "gpio0_12"},
    {"spi0_d0",         "uart2_txd",            "I2C2_SCL",         "ehrpwm0B",         "pr1_uart0_rts_n",      "NULL",                 "EMU3_mux1",            "gpio0_3"},
    {"spi0_sclk",       "uart2_rxd",            "I2C2_SDA",         "ehrpwm0A",         "pr1_uart0_cts_n",      "NULL",                 "EMU2_mux1",            "gpio0_2"},
    {"gpmc_a1",         "gmii2_rxdv",           "rgmii2_rxdv",      "mmc2_dat0",        "gpmc_a17",             "NULL",                 "ehrpwm0_synco",        "gpio1_17"},
    {"uart1_txd",       "mmc2_sdwp",            "dcan1_rx",         "I2C1_SCL",         "NULL",                 "pr1_uart0_txd",        "pr1_pru0_pru_r31_16",  "gpio0_15"},
    {"mcasp0_ahclkx",   "eQEP0_strobe",         "mcasp0_axr3",      "mcasp1_axr1",      "EMU4_mux2",            "pr1_pru0_pru_r30_7",   "pr1_pru0_pru_r31_7",   "gpio3_21"},
    {"uart1_rxd",       "mmc1_sdwp",            "dcan1_tx",         "I2C1_SDA",         "NULL",                 "pr1_uart0_rxd",        "pr1_pru1_pru_r31_16",  "gpio0_14"},
    {"mcasp0_fsr",      "eQEP0B_in",            "mcasp0_axr3",      "mcasp1_fsx",       "EMU2_mux2",            "pr1_pru0_pru_r30_5",   "pr1_pru0_pru_r31_5",   "gpio3_19"},
    {"mcasp0_ahclkr",   "ehrpwm0_synci",        "mcasp0_axr2",      "spi1_cs0",         "eCAP2_in_PWM2_out",    "pr1_pru0_pru_r30_3",   "pr1_pru0_pru_r31_3",   "gpio3_17"},
    {"mcasp0_fsx",      "ehrpwm0B",             "NULL",             "spi1_d0",          "mmc1_sdcd_mux1",       "pr1_pru0_pru_r30_1",   "pr1_pru0_pru_r31_1",   "gpio3_15"},
    {"mcasp0_axr0",     "ehrpwm0_tripzone",     "NULL",             "spi1_d1",          "mmc2_sdcd_mux1",       "pr1_pru0_pru_r30_2",   "pr1_pru0_pru_r31_2",   "gpio3_16"},
    {"mcasp0_aclkx",    "ehrpwm0A",             "NULL",             "spi1_sclk",        "mmc0_sdcd_mux1",       "pr1_pru0_pru_r30_0",   "pr1_pru0_pru_r31_0",   "gpio3_14"},
    {"VADC",            "VADC",                    "VADC",                "VADC",                "VADC",                    "VADC",                    "VADC",                    "VADC"},
    {"AIN4",            "AIN4",                    "AIN4",                "AIN4",                "AIN4",                    "AIN4",                    "AIN4",                    "AIN4"},
    {"AGND",            "AGND",                    "AGND",                "AGND",                "AGND",                    "AGND",                    "AGND",                    "AGND"},
    {"AIN6",            "AIN6",                    "AIN6",                "AIN6",                "AIN6",                    "AIN6",                    "AIN6",                    "AIN6"},
    {"AIN5",            "AIN5",                    "AIN5",                "AIN5",                "AIN5",                    "AIN5",                    "AIN5",                    "AIN5"},
    {"AIN2",            "AIN2",                    "AIN2",                "AIN2",                "AIN2",                    "AIN2",                    "AIN2",                    "AIN2"},
    {"AIN3",            "AIN3",                    "AIN3",                "AIN3",                "AIN3",                    "AIN3",                    "AIN3",                    "AIN3"},
    {"AIN0",            "AIN0",                    "AIN0",                "AIN0",                "AIN0",                    "AIN0",                    "AIN0",                    "AIN0"},
    {"AIN1",            "AIN1",                    "AIN1",                "AIN1",                "AIN1",                    "AIN1",                    "AIN1",                    "AIN1"},
    {"xdma_event_intr1","NULL",                 "tclkin",           "clkout2",          "timer7_mux1",          "pr1_pru0_pru_r31_16",  "EMU3_mux0",            "gpio0_20"},
    {"eCAP0_in_PWM0_out","uart3_txd",           "spi1_cs1",         "pr1_ecap0_ecap_capin_apwm_o","spi1_sclk",        "mmc0_sdwp",            "xdma_event_intr2","gpio0_7"},
    {"GND",                "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"GND",                "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"GND",                "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
    {"GND",                "GND",                    "GND",                "GND",                "GND",                    "GND",                    "GND",                    "GND"},
};

const int gpio_bitfield[92] = {
    0,       0,        1<<6,     1<<7,     1<<2,     1<<3,    1<<2,     1<<3,
    1<<5,    1<<4,     1<<13,    1<<12,    1<<23,    1<<26,   1<<15,    1<<14,
    1<<27,   1<<1,     1<<22,    1<<31,    1<<30,    1<<5,    1<<4,     1<<1,
    1<<0,    1<<29,    1<<22,    1<<24,    1<<23,    1<<25,   1<<10,    1<<11,    
    1<<9,    1<<17,    1<<8,     1<<16,    1<<14,    1<<15,   1<<12,    1<<13,    
    1<<10,   1<<11,    1<<8,     1<<9,     1<<6,     1<<7,
    0,       0,        0,        0,        0,        0,       0,        0,
    0,       0,        1<<30,    1<<28,    1<<31,    1<<18,   1<<16,    1<<19,
    1<<5,    1<<4,     1<<13,    1<<12,    1<<3,     1<<2,    1<<17,    1<<15,
    1<<21,   1<<14,    1<<19,    1<<17,    1<<15,    1<<16,   1<<14,    0,
    0,       0,        0,        0,        0,        0,       0,        0,
    1<<20,   1<<7,     0,        0,        0,        0,
};

const int gpio_bank[92] = {
    -1,      -1,          1,       1,        1,        1,        2,        2,
    2,        2,          1,       1,        0,        0,        1,        1,
    0,        2,          0,       1,        1,        1,        1,        1,
    1,        1,          2,       2,        2,        2,        0,        0,
    0,        2,          0,       2,        2,        2,        2,        2,
    2,        2,          2,       2,        2,        2,               
    -1,      -1,         -1,      -1,       -1,       -1,       -1,       -1,
    -1,      -1,          0,       1,        0,        1,        1,        1,
    0,        0,          0,       0,        0,        0,        1,        0,
    3,        0,          3,       3,        3,        3,        3,       -1,
    -1,      -1,         -1,      -1,       -1,       -1,       -1,       -1,
     0,       0,         -1,      -1,       -1,       -1,
};

const static int gpio_mode_offset[92] ={
    -1,       -1,       0x818,    0x81c,    0x808,    0x80c,    0x890,    0x894,
    0x89c,    0x898,    0x834,    0x830,    0x824,    0x828,    0x83c,    0x838,
    0x82c,    0x88c,    0x820,    0x884,    0x880,    0x814,    0x810,    0x804,
    0x800,    0x87c,    0x8e0,    0x8e8,    0x8e4,    0x8ec,    0x8d8,    0x8dc,
    0x8d4,    0x8cc,    0x8d0,    0x8c8,    0x8c0,    0x8c4,    0x8b8,    0x8bc,
    0x8b0,    0x8b4,    0x8a8,    0x8ac,    0x8a0,    0x8a4,
    -1,       -1,       -1,       -1,       -1,       -1,       -1,       -1,
    -1,       -1,       0x870,    0x878,    0x874,    0x848,    0x840,    0x84c,
    0x95c,    0x958,    0x97c,    0x978,    0x954,    0x950,    0x844,    0x984,
    0x9ac,    0x980,    0x9a4,    0x99c,    0x994,    0x998,    0x990,    -1,
    -1,       -1,       -1,       -1,       -1,       -1,       -1,       -1,
    0x9b4,    0x964,    -1,       -1,       -1,       -1,
};

/* gpio mode definition 
    bit6: 0-Fast, 1-Slow
    bit5: 0-Receiver disabled, 1-Receiver enabled
    bit4: 0-Pulldown, 1-Pullup
    bit3: 0-Pullup/Pulldown enabled
    bit2~0: mux

    input generally: 0x27
    output generally: 0x7
 */

/* flag,connector,pin,dir */
int gpio_used[64][4] = {0};

const unsigned int gpio_base[]={GPIO0_BASE, GPIO1_BASE, GPIO2_BASE, GPIO3_BASE};

volatile void *ctrl_addr = NULL;
volatile void *gpio_addr[4] = {NULL, NULL, NULL, NULL};

int gpio_fd = -1;

void regist_gpio(int connector, int pin, int dir)
{
    int i;
    
    for(i=0;(i<sizeof(gpio_used)/sizeof(gpio_used[0]))&&(gpio_used[i][0] != 0);i++)
    ;

    gpio_used[i][0] = 1;
    gpio_used[i][1] = connector;
    gpio_used[i][2] = pin;
    gpio_used[i][3] = dir;
}

static int varify_gpio()
{
    int i, j;
    
    for(i=0;i<sizeof(gpio_used)/sizeof(gpio_used[0]);i++)
    {
        if(gpio_used[i][0] == 0)
            return 0;
        
        for(j=i+1;j<sizeof(gpio_used)/sizeof(gpio_used[0]);j++)
        {
            if((gpio_used[i][1] == gpio_used[j][1]) && (gpio_used[i][2] == gpio_used[j][2]))
            {
                printf("GPIO Verify Failed: %d-%d\n", gpio_used[i][1], gpio_used[i][2]);
                return -1;
            }
        }
    }
    
    return 0;
}

int gpio_set_dir(int connector, int pin, int dir)
{
    volatile unsigned int* reg;
    int port;

    if((connector != 8) && (connector != 9))
        return -1;
    if((pin < 1) || (pin > 46))
        return -1;
        
    if((dir != 0) && (dir != 1))
        return -1;
        
    port = (connector == 8)?pin-1:pin-1+46;

    if(gpio_bank[port] == -1)
        return -1;

    reg = (unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_OE_OFFSET);

    if (dir == DIR_OUT)
        *reg &= ~(gpio_bitfield[port]);
    else
        *reg |= gpio_bitfield[port];
    
    return 0;
}

int gpio_get_dir(int connector, int pin)
{
    unsigned int value;
    int port, dir;

    if((connector != 8) && (connector != 9))
        return -1;
    
    if((pin < 1) || (pin > 46))
        return -1;
        
    port = (connector == 8)?pin-1:pin+45;

    if(gpio_bank[port] == -1)
        return -1;

    value = *(unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_OE_OFFSET);

    if((value & gpio_bitfield[port]) == 0)
    {
        return DIR_OUT;
    }
    else
    {
        return DIR_IN;
    }
}

int set_pin_high(int connector, int pin)
{
    int port = (connector == 8)?pin-1:pin-1+46;

    *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_SETDATAOUT_OFFSET)) |= gpio_bitfield[port];

    return 0;
}

int set_pin_low(int connector, int pin)
{
    int port = (connector == 8)?pin-1:pin-1+46;

    *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_CLEARDATAOUT_OFFSET)) |= gpio_bitfield[port];

    return 0;
}

int set_pin_irq_mode(int connector, int pin, enum GPIO_IRQ_MODE mode)
{
    int port = (connector == 8)?pin-1:pin-1+46;

    switch(mode)
    {
        case GPIO_IRQ_LOW_LEVEL:
            *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_LEVELDETECT0_OFFSET)) |= gpio_bitfield[port];
            break;
        case GPIO_IRQ_HIGH_LEVEL:
            *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_LEVELDETECT1_OFFSET)) |= gpio_bitfield[port];
            break;
        case GPIO_IRQ_RISING_EDGE:
            *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_RISINGDETECT_OFFSET)) |= gpio_bitfield[port];
            break;
        case GPIO_IRQ_FALLING_EDGE:
            *((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_FALLINGDETECT_OFFSET)) |= gpio_bitfield[port];
            break;
        default:
            return -1;
    }

    return 0;
}

int is_pin_high(int connector, int pin)
{
    int port = (connector == 8)?pin-1:pin-1+46;
    return ((*((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_DATAIN_OFFSET)) & gpio_bitfield[port])!=0);
}

int is_pin_low(int connector, int pin)
{
    int port = (connector == 8)?pin-1:pin-1+46;
    return ((*((unsigned int *)(gpio_addr[gpio_bank[port]] + GPIO_DATAIN_OFFSET)) & gpio_bitfield[port])==0);
}

static void print_all_mode()
{
    int i;

    for(i = 0;i < 92;i++)
    {
        if(i < 46)
            printf("P8[%d]: ", i+1);
        else
            printf("P9[%d]: ", i - 45);

        if(gpio_bank[i] == -1)
            printf("%s\n", gpio_name[i][0]);
        else
        {
            printf("0x%x: 0x%x,%s\n", (gpio_mode_offset[i]), *(unsigned int *)(ctrl_addr + gpio_mode_offset[i]), gpio_name[i][*(unsigned int *)(ctrl_addr + gpio_mode_offset[i]) & 0x7]);
        }

    }
}

static void print_all_dir()
{
    int i;

    printf("################################\n");
    for(i=0;i<46;i++)
    {
        printf("P8_%d,DIR=%d\n", i+1, gpio_get_dir(8, i+1));
    }
    for(i=0;i<46;i++)
    {
        printf("P9_%d,DIR=%d\n", i+1, gpio_get_dir(9, i+1));
    }
    printf("################################\n");
}

int gpio_init()
{
    int i;
    int j;
    
  #ifdef DEBUG
    printf("gpio init ....\n");
  #endif

    if(varify_gpio()<0)
        return -1;

    gpio_fd  = open("/dev/mem", O_RDWR);
    if(gpio_fd < 0)
    {
        printf("Open /dev/gpio_mem failed!\n");
        return -1;
    }
    ctrl_addr = (unsigned int *)mmap(0, CONTROL_LEN, PROT_READ | PROT_WRITE, MAP_SHARED , gpio_fd, CONTROL_MODULE);
    if(ctrl_addr == MAP_FAILED)
    {
        printf("gpio_init: control module mmap failure!, __LINE__=%d\n", __LINE__);
        return -1;
    }

  #ifdef DEBUG
    printf("mmap ctrl address ok   : ");
    printf("%p\n", ctrl_addr);
    
  #endif

  #ifdef DEBUG
    print_all_mode();
  #endif

    for (i = 0;i < 4;i++)
    {
        gpio_addr[i] = mmap(0, GPIOX_LEN, PROT_READ | PROT_WRITE, MAP_SHARED , gpio_fd, gpio_base[i]);
        if(gpio_addr[i] == MAP_FAILED)
        {
            printf("gpio_init: gpio mmap failure!\n");
            return -1;
        }
    }

  #ifdef DEBUG
    for (i = 0;i < 4;i++)
    {
        printf("gpio_addr[%d]=%p\n", i, gpio_addr[i]);
        
        printf("%p,", gpio_addr[i]+GPIO_CTRL_OFFSET);
        printf("GPIO_CTRL_OFFSET=0x%x\n", *(unsigned int *)(gpio_addr[i]+GPIO_CTRL_OFFSET));
        printf("%p,", gpio_addr[i]+GPIO_OE_OFFSET);
        printf("GPIO_OE_OFFSET=0x%x\n", *(unsigned int *)(gpio_addr[i]+GPIO_OE_OFFSET));
        printf("%p,",gpio_addr[i]+GPIO_DATAIN_OFFSET);
        printf("GPIO_DATAIN_OFFSET=0x%x\n", *(unsigned int *)(gpio_addr[i]+GPIO_DATAIN_OFFSET));
        printf("%p,",gpio_addr[i]+GPIO_DATAOUT_OFFSET);
        printf("GPIO_DATAOUT_OFFSET=0x%x\n", *(unsigned int *)(gpio_addr[i]+GPIO_DATAOUT_OFFSET));
        printf("%p,",gpio_addr[i]+GPIO_CLEARDATAOUT_OFFSET);
        printf("GPIO_CLEARDATAOUT_OFFSET=0x%x\n", *(unsigned int *)(gpio_addr[i]+GPIO_CLEARDATAOUT_OFFSET));
        printf("%p,",gpio_addr[i]+GPIO_SETDATAOUT_OFFSET);
        printf("GPIO_SETDATAOUT_OFFSET=0x%x\n\n", *(unsigned int *)(gpio_addr[i]+GPIO_SETDATAOUT_OFFSET));

    }
  #endif

//    print_all_dir();

    for(i = 0;i < sizeof(gpio_used)/sizeof(gpio_used[0]);i++)
    {
        if(gpio_used[i][0] == 0)
            break;

        gpio_set_dir(gpio_used[i][1], gpio_used[i][2], gpio_used[i][3]);
    }
	return 0;
}

int gpio_exit()
{
    if(gpio_fd > 0)
    {
        munmap((void *)ctrl_addr, CONTROL_LEN);
        munmap((void *)&gpio_base[0], GPIOX_LEN);
        munmap((void *)&gpio_base[1], GPIOX_LEN);
        munmap((void *)&gpio_base[2], GPIOX_LEN);
        munmap((void *)&gpio_base[3], GPIOX_LEN);

        close(gpio_fd);
    }
}

void gpio_print_mode(int connector, int pin)
{
    int port, mode;

    port = (connector == 8)?pin-1:pin-1+46;

    if(gpio_bank[port] == -1)
        return;

    mode = *(unsigned int *)(ctrl_addr + gpio_mode_offset[port]);

    printf("PIN(P%d[%d]-%s), mode 0x%x: \n", connector, pin, gpio_name[port][7], mode);
    if(BIT_IS_SET(mode, 6))
        printf("Slower slew rate,");
    else
        printf("Faster slew rate,");

    if(BIT_IS_SET(mode, 5))
        printf("Receiver enabled,");
    else
        printf("Receiver disabled,");

    if(BIT_IS_SET(mode, 4))
        printf("Pullup,");
    else
        printf("Pulldown,");

    if(BIT_IS_SET(mode, 3))
        printf("Pull disabled,");
    else
        printf("Pull enabled,");

    printf("MUX=%d\n", mode & 0x7);
}

#ifdef _cplusplus
}
#endif

#endif

