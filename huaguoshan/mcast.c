#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "pub.h"
#include "common.h"

extern guint d_my_ipaddr;
extern guint g_release_thrds;

static int mc_sockfd = -1;

static unsigned char rcv_buf[BUFLEN];
static unsigned char snd_buf[BUFLEN];
static char hm_name[17];
gpointer multicast_guard()
{
    struct sockaddr_in remote_addr;
    struct in_addr ia;
    int mc_sockfd;
    unsigned int socklen, n;
    struct hostent *m_group;
    struct ip_mreqn m_req;
    struct s_com *rcv_msg, *snd_msg;
    struct s_mc_ipaddr *payload;
    
    mc_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mc_sockfd < 0) 
    {
        g_printf("[MCAST] Socket failed\n");
        return NULL;
    }
    
    memset(&m_req, 0, sizeof(struct ip_mreqn));
    if ((m_group = gethostbyname(MUL_IPADDR)) == NULL) 
    {
        g_printf("[MCAST] Socket failed\n");
        return NULL;
    }
    
    bcopy((void *)m_group->h_addr_list[0], &ia, m_group->h_length);
    bcopy(&ia, &m_req.imr_multiaddr.s_addr, sizeof(struct in_addr));
    
    m_req.imr_address.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(mc_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_req, sizeof(struct ip_mreqn)) == -1) 
    {
        g_printf("[MCAST] Socket failed\n");
        return NULL;
    }
    
    socklen = sizeof(struct sockaddr_in);
    memset(&remote_addr, 0, socklen);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(XZ_PORT2);

    if (inet_pton(AF_INET, MUL_IPADDR, &remote_addr.sin_addr) <= 0) 
    {
        g_printf("[MCAST] Socket failed\n");
        return NULL;
    }
    
    if (bind(mc_sockfd, (struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in)) == -1) 
    {
        g_printf("[MCAST] Socket failed\n");
        return NULL;
    }

    g_printf("[MCAST] ok!\n");
    
    for(;;) 
    {
        if(g_release_thrds)
            break;

        n = recvfrom(mc_sockfd, rcv_buf, BUFLEN, 0, (struct sockaddr *) &remote_addr, &socklen);
        if (n < sizeof(struct s_com)) {
            continue;
        } 
        else 
        {
            memset(snd_buf, 0, sizeof(snd_buf));
            snd_msg = (struct s_com *)snd_buf;
            rcv_msg = (struct s_com *)rcv_buf;
            memset(hm_name, 0, sizeof(hm_name));

            switch(rcv_msg->code)
            {
                case XZ_REQUEST_BASE:
                {
                    snd_msg->code = BA_MC_IPADDR;
                    payload = (struct s_mc_ipaddr *)(snd_msg + sizeof(struct s_com));
                    payload->ipaddr = htonl(d_my_ipaddr);
                    memcpy(hm_name, rcv_buf+sizeof(struct s_com), sizeof(struct s_request_base));

                    /* response with base IP */
                    n = sendto(mc_sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));

                    break;
                }
                case MC_REQUEST_XZ_ADDR:
                {
                    snd_msg->code = BA_REPLY_XZ_ADDR;
                    payload = (struct s_mc_ipaddr *)(snd_msg + sizeof(struct s_com));
                    payload->ipaddr = htonl(d_my_ipaddr);
                    memcpy(hm_name, rcv_buf+sizeof(struct s_com), sizeof(struct s_request_base));

                    n = sendto(mc_sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));

                    break;
                }
                case XZ_REPORTING:
                {
                    printf("receiving reporting from hm\n");
                    break;
                }
                default:
                    break;
            }
        }
        sleep(10);
    }

printf("exit from mcast\n");   
    return NULL;
}


void close_mc_fd()
{
    close(mc_sockfd);
    mc_sockfd = -1;
}

