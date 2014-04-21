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
#include "r_commu.h"

#define MY_NAME "HAMSTER_001"

char         *s_my_ipaddr;
unsigned int d_my_ipaddr;
unsigned int d_base_ipaddr;

struct s_base_info m_base;

/* global */
int m_fd = -1;

static void get_my_addr()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "usb0", IFNAMSIZ-1);

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

    while (bind(sockfd, (struct sockaddr *) &local_addr,sizeof(struct sockaddr_in)) == -1) 
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
        n = recvfrom(sockfd, rcv_buf, BUFLEN, 0, (struct sockaddr *) &remote_addr, &socklen);
        if (n < sizeof(struct s_com)) {
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
                close(sockfd);
                return 0;
            }
            default:
                break;
        }
    }

    return 0;
}

void *communation(void *data)
{
    char rcv_buf[BUFLEN];
    struct hostent *server_host_name;
    int n;
    struct s_com *msg;

    if(wait_for_base() < 0)
    {
        printf("BASE not responsed!\n");
        return NULL;
    }
    
    printf("received response from BASE!\n");

    m_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(m_fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }
    
    memset(&m_base, 0, sizeof(m_base));
    m_base.m_addr.sin_family = AF_INET;
    m_base.m_addr.sin_addr.s_addr = htonl(d_base_ipaddr);
    m_base.m_addr.sin_port = htons(BASE_PORT);
    
    while(connect(m_base.fd, (void *)&m_base.m_addr, sizeof(m_base.m_addr)) < 0)
    {
        sleep(10);
    }

//    set_timer(0, R_TIMER_LOOP, 1000, led_blink, NULL);

    for(;;)
    {    
        n = recv(m_base.fd, (void *)rcv_buf, sizeof(struct s_com), MSG_WAITALL);
        if (n < sizeof(struct s_com)) {
            sleep(10);
            continue;
        }
        printf("Packet Received:");
        msg = (struct s_com *)rcv_buf;
        printf("%d\n", msg->code);
    }
    
    close(m_base.fd);

    return NULL;
}

