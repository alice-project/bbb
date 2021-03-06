#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "pub.h"
#include "common.h"

void BASE_LOG(char *buffer);

extern void set_light1_red();
extern void set_light1_yellow();
extern void set_light1_green();
extern void set_ssonic1_dist(int d);

extern unsigned int d_my_ipaddr;
extern unsigned char *video_frame;
unsigned int frame_size=0;

GSList *g_watchers = NULL;
pthread_t hm_thread[MAX_SUNXINGZHES] = {0};
int g_hm_id = 0;
struct s_hm g_hms[MAX_SUNXINGZHES] = {-1};

int sw_sockfd = -1;

extern void update_video(unsigned char *data, int width, int height);

static gboolean ant_comes(GSocketService    *service,
                          GSocketConnection *connection,
                          GObject           *src_obj,
                          gpointer           user_data)
{
    GSocketAddress     *addr;
    GInetAddress       *ip_addr;
    gchar *str;
    GSocketFamily  ip_type;
    
    addr = g_socket_connection_get_remote_address(connection, NULL);
    ip_addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(addr));
    ip_type = g_inet_address_get_family(ip_addr);

//    gdk_threads_enter();
    
    BASE_LOG("[WATCHER] Here comes an ant: ");
    
    str = g_inet_address_to_string(ip_addr);
    BASE_LOG(str);
    BASE_LOG("\n");
//    gdk_threads_leave();
    
    return TRUE;
}

static unsigned char rcvbuffer[MAX_VIDEO_FRAME_MSG];
void *hm_func(void *data)
{
    int n;
    struct s_com *msg;
    struct s_sxz_video *video;
    struct s_hm *hm = (struct s_hm *)data;
    int slen;
    struct sockaddr_in s_peer;
    struct s_sxz_distance *distance;

    if(hm->fd < 0)
    {
        printf("FD Error\n");
        return;
    }
    BASE_LOG("NEW hm ");
    BASE_LOG(" is comming...\n");

    for(;;)
    {
        if(hm->fd < 0)
            break;
        
        n = recvfrom(hm->fd, rcvbuffer, MAX_VIDEO_FRAME_MSG, 0, (struct sockaddr *)&s_peer, (socklen_t *)&slen);

        if (n <= 0) 
        {
            usleep(10);
            continue;
        }

        msg = (struct s_com *)rcvbuffer;
        switch(msg->code)
        {
            case XZ_REPORTING:
            {
//                g_printf("received HM_REPORTING message from (%d)\n", msg->id);
                break;
            }
            case XZ_DISTANCE:
            {
                distance = (struct s_sxz_distance *)(rcvbuffer + sizeof(struct s_com));
//                g_printf("received HM_DISTANCE message from (%d): ", msg->id);
//                g_printf("ssonic(%d).distance=%d\n", distance->ssonic_id, distance->distance);
set_ssonic1_dist(distance->distance);
if(distance->distance>200)
    set_light1_green();
else if(distance->distance>100)
    set_light1_yellow();
else
    set_light1_red();

                break;
            }
            case XZ_CAMERA:
            {
                video = (struct s_sxz_video *)(rcvbuffer+sizeof(struct s_com));
                g_printf("received HM_CAMERA message from (%d), size=%d\n", msg->id, video->size);
                memcpy(video_frame, video->data, video->size);
                frame_size = video->size;
//                update_video(video_frame, 640, 480);
                break;
            }
        }
    }

    pthread_exit(NULL);
    g_hm_id--;
    return;
}

gpointer start_watching()
{
    int i;
    struct sockaddr_in s_addr;
    
    sw_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(sw_sockfd < 0)
    {
        BASE_LOG("[WATCHER] Socket failed\n");
        return NULL;
    }

    for(i=0;i<MAX_SUNXINGZHES;i++)
    {
        g_hms[i].fd = -1;
    }
    
    memset((void *)&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(d_my_ipaddr);
    s_addr.sin_port = htons(XZ_PORT1);

    if(bind(sw_sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        BASE_LOG("[WATCHER] Bind failed\n");
printf("d_my_ipaddr=%d.%d.%d.%d\n", d_my_ipaddr>>24, (d_my_ipaddr>>16)&255,(d_my_ipaddr>>8)&255,d_my_ipaddr&255);
        perror("BIND FAILED\n");
        return NULL;
    }
    
    if(listen(sw_sockfd, SOMAXCONN) < 0)
    {
        BASE_LOG("[WATCHER] Listen failed\n");
        return NULL;
    }

    BASE_LOG("[WATCHER] Base is listening...!\n");
    
    for(;;)
    {
        memset(&g_hms[g_hm_id].c_addr, 0, sizeof(g_hms[g_hm_id].c_addr));
        socklen_t len = sizeof(g_hms[g_hm_id].c_addr);
        g_hms[g_hm_id].fd = accept(sw_sockfd, (struct sockaddr *)&g_hms[g_hm_id].c_addr, &len);
        if(g_hms[g_hm_id].fd < 0)
        {
            BASE_LOG("[WATCHER] Accept failed\n");
        }
        else
        {
            if(pthread_create(&hm_thread[g_hm_id], NULL, hm_func, &g_hms[g_hm_id]) < 0)
            {
                BASE_LOG("[WATCHER] pthread_create failed\n");
                close(g_hms[g_hm_id].fd);
                g_hms[g_hm_id].fd = -1;
                close(sw_sockfd);
                sw_sockfd = -1;
                return NULL;
            }
	        g_hm_id++;
        }
    }
    
    for(i=0;i<g_hm_id;i++)
        pthread_join(hm_thread[i], 0);

    close(sw_sockfd);
    sw_sockfd = -1;
    
    return NULL;
}

void close_sw_fd()
{
    close(sw_sockfd);
    sw_sockfd = -1;
}

