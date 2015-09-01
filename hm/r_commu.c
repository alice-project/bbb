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
u_int32 d_my_ipaddr;
u_int32 d_base_ipaddr;

struct s_base_info m_base;

MSG_HANDLER cmd_cb[64];

int regist_cmd_cb(u_int8 id, MSG_HANDLER cb)
{
    if(id > 63)
        return -1;

    if(cmd_cb[id]!=NULL)
        return -2;

    if(cb==NULL)
        return -3;

    cmd_cb[id] = cb;
}

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
    u_int32 socklen, n;
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
    remote_addr.sin_port = htons(HM_PORT2);

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
        while (n < sizeof(struct s_com)) {
            /* keep requesting unless received response from BASE */
            sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));
            sleep(1);
	        n = recvfrom(sockfd, rcv_buf, BUFLEN, MSG_DONTWAIT, (struct sockaddr *) &remote_addr, &socklen);
            continue;
        }
        rcv_msg = (struct s_com *)rcv_buf;
        switch(rcv_msg->code)
        {
            case BA_MC_IPADDR:
            {
                rcv_payload = (struct s_mc_ipaddr *)(rcv_msg + sizeof(struct s_com));
                d_base_ipaddr =  htonl(rcv_payload->ipaddr);
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
    free(snd_buf);

    return 0;
}

int report_distance_info(struct s_hm_distance *data)
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
    msg->code = HM_DISTANCE;
    msg->id = 0; /* temporary set */
    msg->flags = 0;

    memcpy(snd_buf+sizeof(struct s_com), data, sizeof(struct s_hm_distance));
    
    n = sendto(m_base.fd, snd_buf, BUFLEN, 0, (struct sockaddr *) &m_base.m_addr, sizeof(struct sockaddr_in));
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
    m_base.m_addr.sin_port = htons(HM_PORT1);
    
    while(connect(m_base.fd, (struct sockaddr *)&m_base.m_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect timeout!\n");
        sleep(10);
    }

    for(;;)
    {    
        n = recv(m_base.fd, (void *)rcv_buf, BUFLEN, MSG_WAITALL);
        if (n < sizeof(struct s_com)) {
            sleep(10);
            continue;
        }
        msg = (struct s_com *)rcv_buf;

        if(msg->code > 63)
        {
            printf("UNKNOWN command(%d) is received!\n", msg->code);
            continue;
        }
        if(cmd_cb[msg->code] == NULL)
        {
            printf("UNDEFINED command(%d) is received!\n", msg->code);
            continue;
        }
        
        cmd_cb[msg->code](rcv_buf + sizeof(struct s_com));
/*            case BA_LIGHT_CMD:
            {
                //printf("received  BA_LIGHT_CMD command from base\n");
                struct s_base_light *light_cmd = (struct s_base_light *)(rcv_buf + sizeof(struct s_com));
                if(light_cmd->on_off == BA_LIGHT_ON) {
                    set_led_on();
                }
                else {
                    set_led_off();
                }
                break;
            }

            case BA_CAMERA_CMD:
            {
                struct s_base_camera_cmd *camera_cmd = (struct s_base_camera_cmd *)(rcv_buf + sizeof(struct s_com));
                if(camera_cmd->on_off == 1) {
                    set_camera_rotate(1);
                }
                else {
                    set_camera_rotate(1);
                }
                break;
            }
            case BA_GC_SETTINGS:
            {
                //printf("received  BA_GC_SETTINGS command from base\n");
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
            case BA_SERVO_CMD:
            {
                struct s_base_servo_cmd *servo_cmd = (struct s_base_servo_cmd *)(rcv_buf + sizeof(struct s_com));

                if(servo_cmd->cmd == 1)  // 正转
                    servo_rotate_to(servo_cmd->id, servo_cmd->angle);
                else
                    servo_rotate_to(servo_cmd->id, -1*servo_cmd->angle);

                break;
            }
*/
    }
    
    close(m_base.fd);

    return NULL;
}

