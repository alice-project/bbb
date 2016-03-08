#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <netdb.h>

#include "pub.h"
#include "common.h"
#include "gpio.h"
#include "r_event.h"
#include "r_message.h"
#include "r_timer.h"
#include "r_usonic.h"
#include "r_motor.h"
#include "r_led.h"
#include "r_commu.h"

int hm_test(void *d)
{
    if(is_pin_low(9,11))
        printf("LOW\n");
    else
        printf("HIGH\n");
 
    return 0;
}

int parser_test_cmd()
{
static int i=0;
if((i%2)==0)
set_pin_high(9,14);
else
set_pin_low(9,14);

i++;
return 0;
}

