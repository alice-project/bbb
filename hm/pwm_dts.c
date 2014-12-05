#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "pwm.h"

unsigned char pwm_name[255];

/* initial configure for PWMs */
struct pwm_exp hm_pwm[] = 
{
    {8, 13, "/sys/devices/ocp.3/left_wheel.13", 1500000, 20000000, 0},
    {8, 19, "/sys/devices/ocp.3/right_wheel.14", 1500000, 20000000, 0},
    {9, 22, "/sys/devices/ocp.3/servo_motor_pwm.15", 1500000, 20000000, 0},
};

int pwm_init()
{
    int i;
    char buffer[20];
    FILE *fp;

    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        memset(pwm_name, 0, sizeof(pwm_name));
        sprintf(pwm_name, "%s/run", hm_pwm[i].path_and_name);
        fp = fopen(pwm_name, "w");
        if(fp != NULL) {
            fputs("0", fp);
            fclose(fp);
        }

        memset(pwm_name, 0, sizeof(pwm_name));
        sprintf(pwm_name, "%s/duty", hm_pwm[i].path_and_name);
        fp = fopen(pwm_name, "w");
        if(fp != NULL) {
            sprintf(buffer, "%d", hm_pwm[i].duty);
            fputs(buffer, fp);
            fclose(fp);
        }

        memset(pwm_name, 0, sizeof(pwm_name));
        sprintf(pwm_name, "%s/period", hm_pwm[i].path_and_name);
        fp = fopen(pwm_name, "w");
        if(fp != NULL) {
            sprintf(buffer, "%d", hm_pwm[i].period);
            fputs(buffer, fp);
            fclose(fp);
        }

        memset(pwm_name, 0, sizeof(pwm_name));
        sprintf(pwm_name, "%s/polarity", hm_pwm[i].path_and_name);
        fp = fopen(pwm_name, "w");
        if(fp != NULL) {
            sprintf(buffer, "%d", hm_pwm[i].polarity);
            fputs(buffer, fp);
            fclose(fp);
        }
    }
}

int pwm_set_duty(int connector, int pin, unsigned int duty)
{
    FILE *fp;
    int i;
    char buffer[20];
    unsigned long duty_cycle=0;
    
    if (duty < 0.0 || duty > 100.0)
        return -1;
    
    /* find the specified pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/duty", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;

    duty_cycle = hm_pwm[i].period * duty;
    hm_pwm[i].duty = duty_cycle;

    sprintf(buffer, "%lu", duty_cycle);
    fputs(buffer, fp);
    
    fclose(fp);

    return 0;
}

int pwm_set_duty_cycle(int connector, int pin, unsigned int duty)
{
    FILE *fp;
    int i;
    char buffer[20];
    
    /* find the specified pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/duty", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;

    hm_pwm[i].duty = duty;

    sprintf(buffer, "%lu", duty);
    fputs(buffer, fp);
    
    fclose(fp);

    return 0;
}

int pwm_get_duty(int connector, int pin)
{
    int i;
    
    /* find the specified pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;

    return hm_pwm[i].duty;
}

int pwm_incr_duty(int connector, int pin, unsigned int var)
{
    int duty = pwm_get_duty(connector, pin);
    pwm_set_duty(connector, pin, duty);
    return 0;
}

int pwm_set_period(int connector, int pin, unsigned int period)
{
    FILE *fp;
    int i;
    char buffer[20];
    
    /* find the specified pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/period", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;

    hm_pwm[i].period = period;

    sprintf(buffer, "%lu", period);
    fputs(buffer, fp);
    
    fclose(fp);

    return 0;
}

int pwm_get_period(int connector, int pin)
{
    int i;
    
    /* find the specified pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    return hm_pwm[i].period;
}

int pwm_incr_freq(int connector, int pin, unsigned int var)
{
    int freq = pwm_get_period(connector, pin);
    pwm_set_period(connector, pin, freq);

    return 0;
}


int pwm_run(int connector, int pin)
{
    FILE *fp;
    int i;
    
    /* find the pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/run", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;

    fputs("1", fp);
    
    fclose(fp);
    return 0;
}

int pwm_stop(int connector, int pin)
{
    FILE *fp;
    int i;
    
    /* find the pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/run", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;

    fputs("0", fp);
    
    fclose(fp);
    return 0;
}

int pwm_set_polarity(int connector, int pin, int polarity)
{
    FILE *fp;
    int i;
    
    /* find the pwm */
    for(i=0;i<sizeof(hm_pwm)/sizeof(struct pwm_exp);i++) {
        if((hm_pwm[i].connector==connector)&&(hm_pwm[i].pin==pin))
            break;
    }

    if(i == sizeof(hm_pwm)/sizeof(struct pwm_exp))
        return -1;
    
    memset(pwm_name, 0, sizeof(pwm_name));
    sprintf(pwm_name, "%s/polarity", hm_pwm[i].path_and_name);
    fp = fopen(pwm_name, "w");
    if(fp == NULL)
        return -1;
    
    if(polarity == 1)
        fputs("1", fp);
    else
        fputs("0", fp);
    
    fclose(fp);
    return 0;
}

