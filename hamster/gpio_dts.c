/* using device-tree to control GPIO */
#include "gpio.h"

#ifdef GPIO_USING_DTS

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


const static char *gpio_dts_dir[92] = {
    "",
    "",
    "/sys/class/gpio/gpio38/direction",
    "/sys/class/gpio/gpio39/direction",
    "/sys/class/gpio/gpio34/direction",
    "/sys/class/gpio/gpio35/direction",
    "/sys/class/gpio/gpio66/direction",
    "/sys/class/gpio/gpio67/direction",
    "/sys/class/gpio/gpio69/direction",
    "/sys/class/gpio/gpio68/direction",
    "/sys/class/gpio/gpio45/direction",
    "/sys/class/gpio/gpio44/direction",
    "/sys/class/gpio/gpio23/direction",
    "/sys/class/gpio/gpio26/direction",
    "/sys/class/gpio/gpio47/direction",
    "/sys/class/gpio/gpio46/direction",
    "/sys/class/gpio/gpio27/direction",
    "/sys/class/gpio/gpio65/direction",
    "/sys/class/gpio/gpio22/direction",
    "/sys/class/gpio/gpio63/direction",
    "/sys/class/gpio/gpio62/direction",
    "/sys/class/gpio/gpio37/direction",
    "/sys/class/gpio/gpio36/direction",
    "/sys/class/gpio/gpio33/direction",
    "/sys/class/gpio/gpio32/direction",
    "/sys/class/gpio/gpio61/direction",
    "/sys/class/gpio/gpio86/direction",
    "/sys/class/gpio/gpio88/direction",
    "/sys/class/gpio/gpio87/direction",
    "/sys/class/gpio/gpio89/direction",
    "/sys/class/gpio/gpio10/direction",
    "/sys/class/gpio/gpio11/direction",
    "/sys/class/gpio/gpio9/direction",
    "/sys/class/gpio/gpio81/direction",
    "/sys/class/gpio/gpio8/direction",
    "/sys/class/gpio/gpio80/direction",
    "/sys/class/gpio/gpio78/direction",
    "/sys/class/gpio/gpio79/direction",
    "/sys/class/gpio/gpio76/direction",
    "/sys/class/gpio/gpio77/direction",
    "/sys/class/gpio/gpio74/direction",
    "/sys/class/gpio/gpio75/direction",
    "/sys/class/gpio/gpio72/direction",
    "/sys/class/gpio/gpio73/direction",
    "/sys/class/gpio/gpio70/direction",
    "/sys/class/gpio/gpio71/direction",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "/sys/class/gpio/gpio30/direction",
    "/sys/class/gpio/gpio60/direction",
    "/sys/class/gpio/gpio31/direction",
    "/sys/class/gpio/gpio50/direction",
    "/sys/class/gpio/gpio48/direction",
    "/sys/class/gpio/gpio51/direction",
    "/sys/class/gpio/gpio5/direction",
    "/sys/class/gpio/gpio4/direction",
    "/sys/class/gpio/gpio13/direction",
    "/sys/class/gpio/gpio12/direction",
    "/sys/class/gpio/gpio3/direction",
    "/sys/class/gpio/gpio2/direction",
    "/sys/class/gpio/gpio49/direction",
    "/sys/class/gpio/gpio15/direction",
    "/sys/class/gpio/gpio117/direction",
    "/sys/class/gpio/gpio14/direction",
    "/sys/class/gpio/gpio115/direction",
    "/sys/class/gpio/gpio113/direction",
    "/sys/class/gpio/gpio111/direction",
    "/sys/class/gpio/gpio112/direction",
    "/sys/class/gpio/gpio110/direction",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "/sys/class/gpio/gpio20/direction",
    "/sys/class/gpio/gpio7/direction",
    "",
    "",
    "",
    ""
};

const static char *gpio_dts_value[92] = {
    "",
    "",
    "/sys/class/gpio/gpio38/value",
    "/sys/class/gpio/gpio39/value",
    "/sys/class/gpio/gpio34/value",
    "/sys/class/gpio/gpio35/value",
    "/sys/class/gpio/gpio66/value",
    "/sys/class/gpio/gpio67/value",
    "/sys/class/gpio/gpio69/value",
    "/sys/class/gpio/gpio68/value",
    "/sys/class/gpio/gpio45/value",
    "/sys/class/gpio/gpio44/value",
    "/sys/class/gpio/gpio23/value",
    "/sys/class/gpio/gpio26/value",
    "/sys/class/gpio/gpio47/value",
    "/sys/class/gpio/gpio46/value",
    "/sys/class/gpio/gpio27/value",
    "/sys/class/gpio/gpio65/value",
    "/sys/class/gpio/gpio22/value",
    "/sys/class/gpio/gpio63/value",
    "/sys/class/gpio/gpio62/value",
    "/sys/class/gpio/gpio37/value",
    "/sys/class/gpio/gpio36/value",
    "/sys/class/gpio/gpio33/value",
    "/sys/class/gpio/gpio32/value",
    "/sys/class/gpio/gpio61/value",
    "/sys/class/gpio/gpio86/value",
    "/sys/class/gpio/gpio88/value",
    "/sys/class/gpio/gpio87/value",
    "/sys/class/gpio/gpio89/value",
    "/sys/class/gpio/gpio10/value",
    "/sys/class/gpio/gpio11/value",
    "/sys/class/gpio/gpio9/value",
    "/sys/class/gpio/gpio81/value",
    "/sys/class/gpio/gpio8/value",
    "/sys/class/gpio/gpio80/value",
    "/sys/class/gpio/gpio78/value",
    "/sys/class/gpio/gpio79/value",
    "/sys/class/gpio/gpio76/value",
    "/sys/class/gpio/gpio77/value",
    "/sys/class/gpio/gpio74/value",
    "/sys/class/gpio/gpio75/value",
    "/sys/class/gpio/gpio72/value",
    "/sys/class/gpio/gpio73/value",
    "/sys/class/gpio/gpio70/value",
    "/sys/class/gpio/gpio71/value",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "/sys/class/gpio/gpio30/value",
    "/sys/class/gpio/gpio60/value",
    "/sys/class/gpio/gpio31/value",
    "/sys/class/gpio/gpio50/value",
    "/sys/class/gpio/gpio48/value",
    "/sys/class/gpio/gpio51/value",
    "/sys/class/gpio/gpio5/value",
    "/sys/class/gpio/gpio4/value",
    "/sys/class/gpio/gpio13/value",
    "/sys/class/gpio/gpio12/value",
    "/sys/class/gpio/gpio3/value",
    "/sys/class/gpio/gpio2/value",
    "/sys/class/gpio/gpio49/value",
    "/sys/class/gpio/gpio15/value",
    "/sys/class/gpio/gpio117/value",
    "/sys/class/gpio/gpio14/value",
    "/sys/class/gpio/gpio115/value",
    "/sys/class/gpio/gpio113/value",
    "/sys/class/gpio/gpio111/value",
    "/sys/class/gpio/gpio112/value",
    "/sys/class/gpio/gpio110/value",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "/sys/class/gpio/gpio20/value",
    "/sys/class/gpio/gpio7/value",
    "",
    "",
    "",
    "",
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
                printf("GPIO Varify Failed: %d-%d\n", i, j);
                return -1;
            }
        }
    }
    
    return 0;
}

int gpio_set_dir(int connector, int pin, int dir)
{
    FILE *fp;
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

    fp = fopen(gpio_dts_dir[gpio_bank[port]*32+gpio_bitfield[port]], "w");
    if(fp == NULL)
    {
        return -1;
    }

    if (dir == DIR_OUT)
        fputs("out", fp);
    else
        fputs("in", fp);
    
    fclose(fp);
    return 0;
}

int gpio_get_dir(int connector, int pin)
{
    char str[8] = {0};
    int port;
    FILE *fp;

    if((connector != 8) && (connector != 9))
        return -1;
    
    if((pin < 1) || (pin > 46))
        return -1;
        
    port = (connector == 8)?pin-1:pin-1+46;

    fp = fopen(gpio_dts_dir[gpio_bank[port]*32+gpio_bitfield[port]], "r");
    if(fp == NULL)
    {
        return -1;
    }

    fgets(str, 4, fp);
    fclose(fp);

    if(strcmp(str, "in") == 0)
        return 1;
    else
        return 0;
}

int set_pin_high(int connector, int pin)
{
    FILE *fp;
    int port = (connector == 8)?pin-1:pin-1+46;

    fp = fopen(gpio_dts_value[gpio_bank[port]*32+gpio_bitfield[port]], "w");
    if(fp == NULL)
    {
        return -1;
    }

    fputs("1", fp);

    fclose(fp);

    return 0;
}

int set_pin_low(int connector, int pin)
{
    FILE *fp;
    int port = (connector == 8)?pin-1:pin-1+46;

    fp = fopen(gpio_dts_value[gpio_bank[port]*32+gpio_bitfield[port]], "w");
    if(fp == NULL)
    {
        return -1;
    }

    fputs("0", fp);

    fclose(fp);

    return 0;
}

int is_pin_high(int connector, int pin)
{
    FILE *fp;
    char str[4] = {0};
    int port = (connector == 8)?pin-1:pin-1+46;

    fp = fopen(gpio_dts_value[gpio_bank[port]*32+gpio_bitfield[port]], "r");
    if(fp == NULL)
    {
        return -1;
    }

    fgets(str, 2, fp);

    fclose(fp);

    if(strcmp(str, "1") == 0)
        return 1;
    else
        return 0;
}

int is_pin_low(int connector, int pin)
{
    FILE *fp;
    char str[4] = {0};
    int port = (connector == 8)?pin-1:pin-1+46;

    fp = fopen(gpio_dts_value[gpio_bank[port]*32+gpio_bitfield[port]], "r");
    if(fp == NULL)
    {
        return -1;
    }

    fgets(str, 2, fp);

    fclose(fp);

    if(strcmp(str, "0") == 0)
        return 1;
    else
        return 0;
}

static print_all_mode()
{

}

static void print_all_dir()
{
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

    for(i = 0;i < sizeof(gpio_used)/sizeof(gpio_used[0]);i++)
    {
        if(gpio_used[i][0] == 0)
            break;

        gpio_set_dir(gpio_used[i][1], gpio_used[i][2], gpio_used[i][3]);
    }

    print_all_dir();

}

int gpio_exit()
{
}

void gpio_print_mode(int connector, int pin)
{
}

#endif

