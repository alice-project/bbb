#ifndef __PWM_DTS_H__
#define __PWM_DTS_H__

struct pwm_exp {
    int connector;
    int pin;
    unsigned char path_and_name[255];
    int duty;
    int period;
    int polarity;
};

int pwm_incr_freq(int connector, int pin, unsigned int var);
int pwm_incr_duty(int connector, int pin, unsigned int var);
int pwm_stop(int connector, int pin);
int pwm_run(int connector, int pin);
int pwm_set_duty(int connector, int pin, unsigned int duty);
int pwm_set_duty_cycle(int connector, int pin, unsigned int duty);
int pwm_set_period(int connector, int pin, unsigned int period);
int pwm_set_polarity(int connector, int pin, int polarity);


#endif

