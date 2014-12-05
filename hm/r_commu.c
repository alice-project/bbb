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
#include "pwm.h"
#include "r_event.h"
#include "r_message.h"
#include "r_timer.h"
#include "r_usonic.h"
#include "r_motor.h"
#include "r_led.h"
#include "r_commu.h"

char         *s_my_ipaddr;
unsigned int d_my_ipaddr;
unsigned int d_base_ipaddr;

struct s_base_info m_base;

static void get_my_addr()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
        return;

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    s_my_ipaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    d_my_ipaddr = inet_network(s_my_ipaddr);

    close(fd);
}

static int wait_for_base()
{
    struct sockaddr_in remote_addr, local_addr;
    
    int sockfd;
    char rcv_buf[BUFLEN];
    char snd_buf[BUFLEN];
    unsigned int socklen, n;
    struct s_com *rcv_msg, *snd_msg;
    struct s_request_base *snd_payload;
    struct s_mc_ipaddr    *rcv_payload;

    get_my_addr();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        printf("socket creating error\n");
        sleep(10);
    }
    socklen = sizeof(struct sockaddr_in);

    memset(&remote_addr, 0, socklen);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(BASE_PORT);

    while (inet_pton(AF_INET, MUL_IPADDR, &remote_addr.sin_addr) <= 0) 
    {
        printf("wrong group address!\n");
        sleep(10);
    }
    
    memset(&local_addr, 0, socklen);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(23456);
    
    while (inet_pton(AF_INET, s_my_ipaddr, &local_addr.sin_addr) <= 0) 
    {
        printf("self ip address error!\n");
        sleep(10);
    }

    while (bind(sockfd, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) == -1) 
    {
        printf("Bind error\n");
        sleep(10);
    }
    
    bzero(snd_buf, BUFLEN);
    rcv_msg = (struct s_com *)snd_buf;
    rcv_msg->code = HM_REQUEST_BASE;
    snd_payload = (struct s_request_base *)(snd_buf + sizeof(struct s_com));
    memcpy(snd_payload->name, MY_NAME, sizeof(MY_NAME));

    while (sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in)) < 0) 
    {
        perror("sendto error");
        sleep(10);
    }

    for (;;) 
    {
        bzero(rcv_buf, BUFLEN);
        n = recvfrom(sockfd, rcv_buf, BUFLEN, MSG_DONTWAIT, (struct sockaddr *) &remote_addr, &socklen);
        if (n < sizeof(struct s_com)) {
            /* keep requesting unless received response from BASE */
            sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));
            sleep(10);
            continue;
        }

        rcv_msg = (struct s_com *)rcv_buf;
        switch(rcv_msg->code)
        {
            case BA_MC_IPADDR:
            {
                rcv_payload = (struct s_mc_ipaddr *)(rcv_msg + sizeof(struct s_com));
                d_base_ipaddr =  htonl(rcv_payload->ipaddr);
printf("BASE IP is 0x%x\n", d_base_ipaddr);
                close(sockfd);
                return 0;
            }
            default:
                break;
        }
    }

    return 0;
}

int regular_info(void *data)
{
    char *snd_buf;
    struct s_com *msg;
    int n;
    
    snd_buf = (char *)malloc(sizeof(char)*BUFLEN);
    if(snd_buf == NULL)
    {
        return -1;
    }
    memset(snd_buf, 0, sizeof(char)*BUFLEN);
    msg = (struct s_com *)snd_buf;
    msg->code = HM_REPORTING;
    msg->id = 0; /* temporary set */
    msg->flags = 0;
    
    n = sendto(m_base.fd, snd_buf, BUFLEN, 0, (struct sockaddr *) &m_base.m_addr, sizeof(struct sockaddr_in));
printf("regular reporting to base: %d!\n", n);

    free(snd_buf);
    return 0;
}

void *commu_proc(void *data)
{
    char rcv_buf[BUFLEN];
    struct hostent *server_host_name;
    int n;
    struct s_com *msg;
    printf("communication thread started!\n");
    if(wait_for_base() < 0)
    {
        printf("BASE not responsed!\n");
        return NULL;
    }
    
    printf("received response from BASE!\n");
    printf("BASE ip is 0x%x\n", d_base_ipaddr);

    m_base.fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(m_base.fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }
    
    memset(&m_base.m_addr, 0, sizeof(m_base.m_addr));
    m_base.m_addr.sin_family = AF_INET;
    m_base.m_addr.sin_addr.s_addr = htonl(d_base_ipaddr);
    m_base.m_addr.sin_port = htons(BASE_PORT);
    
    while(connect(m_base.fd, (struct sockaddr *)&m_base.m_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect timeout!\n");
        sleep(10);
    }
    printf("start timer\n");
    set_timer(0, R_TIMER_LOOP, 10000, regular_info, NULL);

    for(;;)
    {    
        n = recv(m_base.fd, (void *)rcv_buf, BUFLEN, MSG_WAITALL);
        if (n < sizeof(struct s_com)) {
            sleep(10);
            continue;
        }
        msg = (struct s_com *)rcv_buf;
        switch(msg->code)
        {
            case BA_MOTION_CMD:
            {
                struct s_base_motion *motion_cmd = (struct s_base_motion *)(rcv_buf + sizeof(struct s_com));
                printf("received  BA_MOTION_CMD command from base\n");
                parser_motion_cmd(motion_cmd);
                break;
            }
            case BA_LIGHT_CMD:
            {
                printf("received  BA_LIGHT_CMD command from base\n");
                struct s_base_light *light_cmd = (struct s_base_light *)(rcv_buf + sizeof(struct s_com));
                if(light_cmd->on_off == BA_LIGHT_ON) {
                    set_led_on();
                }
                else {
                    set_led_off();
                }
                break;
            }
            case BA_TEST_CMD:
            {
                parser_test_cmd();
                break;
            }
            case BA_PWM_DUTY_CMD:
            {
                struct s_base_pwm_duty *duty_cmd = (struct s_base_pwm_duty *)(rcv_buf + sizeof(struct s_com));
                if(duty_cmd->cmd == 0)
                    pwm_incr_duty(8,13,10);
                else
                    pwm_incr_duty(8,13,-10);
                break;
            }
            case BA_PWM_FREQ_CMD:
            {
                struct s_base_pwm_freq *freq_cmd = (struct s_base_pwm_freq *)(rcv_buf + sizeof(struct s_com));
                if(freq_cmd->cmd == 0)
                    pwm_incr_freq(8,13,1000);
                else
                    pwm_incr_freq(8,13,-1000);
                break;
            }
            default:
            {
                printf("UNKNOWN command(%d) is received!\n", msg->code);
                break;
            }
        }
    }
    
    close(m_base.fd);

    return NULL;
}

