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

extern void set_light1_red();
extern void set_light1_yellow();
extern void set_light1_green();
extern void set_ssonic1_dist(int d);

extern unsigned int d_my_ipaddr;
extern unsigned char *video_frame;
unsigned int frame_size=0;

GSList *g_watchers = NULL;
pthread_t sxz_thread[MAX_SUNXINGZHES] = {0};
int g_sxz_id = 0;
struct s_sxz g_sxz[MAX_SUNXINGZHES] = {-1};

int sw_sockfd = -1;
extern guint g_release_thrds;

extern void update_video(unsigned char *data, int width, int height);

static gboolean xingzhe_comes(GSocketService    *service,
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
    
    g_printf("[WATCHER] Here comes an monkey: ");
    
    str = g_inet_address_to_string(ip_addr);
    g_printf(str);
    g_printf("\n");
//    gdk_threads_leave();
    
    return TRUE;
}

static unsigned char rcvbuffer[MAX_VIDEO_FRAME_MSG];
void *sxz_func(void *data)
{
    int n;
    struct s_com *msg;
    struct s_sxz_video *video;
    struct s_sxz *sxz = (struct s_sxz *)data;
    int slen;
    struct sockaddr_in s_peer;
    struct s_sxz_distance *distance;

    if(sxz->fd < 0)
    {
        printf("FD Error\n");
        return;
    }
    g_printf("NEW sun xingzhe ");
    g_printf(" is comming...\n");

    for(;;)
    {
        if(sxz->fd < 0)
            break;
        
        n = recvfrom(sxz->fd, rcvbuffer, MAX_VIDEO_FRAME_MSG, 0, (struct sockaddr *)&s_peer, (socklen_t *)&slen);

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
//                g_printf("received XZ_REPORTING message from (%d)\n", msg->id);
                break;
            }
            case XZ_DISTANCE:
            {
                distance = (struct s_sxz_distance *)(rcvbuffer + sizeof(struct s_com));
//                g_printf("received XZ_DISTANCE message from (%d): ", msg->id);
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
                g_printf("received XZ_CAMERA message from (%d), size=%d\n", msg->id, video->size);
                memcpy(video_frame, video->data, video->size);
                frame_size = video->size;
//                update_video(video_frame, 640, 480);
                break;
            }
        }
    }

    pthread_exit(NULL);
    g_sxz_id--;
    return;
}

gpointer start_watching()
{
    int i;
    struct sockaddr_in s_addr;
    
    sw_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(sw_sockfd < 0)
    {
        g_printf("[WATCHER] Socket failed\n");
        return NULL;
    }

    for(i=0;i<MAX_SUNXINGZHES;i++)
    {
        g_sxz[i].fd = -1;
    }
    
    memset((void *)&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(d_my_ipaddr);
    s_addr.sin_port = htons(XZ_PORT1);

    if(bind(sw_sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        g_printf("[WATCHER] Bind failed\n");
        g_printf("d_my_ipaddr=%d.%d.%d.%d\n", d_my_ipaddr>>24, (d_my_ipaddr>>16)&255,(d_my_ipaddr>>8)&255,d_my_ipaddr&255);
        perror("BIND FAILED\n");
        return NULL;
    }
    
    if(listen(sw_sockfd, SOMAXCONN) < 0)
    {
        g_printf("[WATCHER] Listen failed\n");
        return NULL;
    }

    g_printf("[WATCHER] Base is listening...!\n");
    
    for(;;)
    {
        if(g_release_thrds)
            break;

        memset(&g_sxz[g_sxz_id].c_addr, 0, sizeof(g_sxz[g_sxz_id].c_addr));
        socklen_t len = sizeof(g_sxz[g_sxz_id].c_addr);
        g_sxz[g_sxz_id].fd = accept(sw_sockfd, (struct sockaddr *)&g_sxz[g_sxz_id].c_addr, &len);
        if(g_sxz[g_sxz_id].fd < 0)
        {
            g_printf("[WATCHER] Accept failed\n");
        }
        else
        {
            if(pthread_create(&sxz_thread[g_sxz_id], NULL, sxz_func, &g_sxz[g_sxz_id]) < 0)
            {
                g_printf("[WATCHER] pthread_create failed\n");
                close(g_sxz[g_sxz_id].fd);
                g_sxz[g_sxz_id].fd = -1;
                close(sw_sockfd);
                sw_sockfd = -1;
                return NULL;
            }
	        g_sxz_id++;
        }
    }
    
    for(i=0;i<g_sxz_id;i++)
        pthread_join(sxz_thread[i], 0);

    close(sw_sockfd);
    sw_sockfd = -1;

printf("exit from watcher\n");    
    return NULL;
}

void close_sw_fd()
{
    close(sw_sockfd);
    sw_sockfd = -1;
}

