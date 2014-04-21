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

extern unsigned int d_my_ipaddr;

GSList *g_watchers = NULL;
pthread_t hm_thread[MAX_HAMTERS] = {0};
int g_hm_id = 0;
struct s_hm g_hms[MAX_HAMTERS] = {-1};

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
    gdk_threads_enter();
    
    g_printf("Here comes an ant: ");
    
    str = g_inet_address_to_string(ip_addr);
    g_printf("%s\n", str);
    gdk_threads_leave();
    
    return TRUE;
}

char buffer[1500];
void *hm_func(void *data)
{
    int fd = *(int *)data;
    int n, num;
    if(fd < 0)
    {
        printf("FD Error\n");
        return;
    }
    printf("NEW hm %d is comming...\n", fd);

    while((num = recv(fd, (void *)buffer, 1500, MSG_WAITALL)) <= 0);
    printf("Packet Received!\n");
    for(n=0;n<num;n++)
    {
        printf("%c", buffer[n]);
    }
    printf("\n");

    close(fd);
    pthread_exit(NULL);
    g_hm_id--;
    return;
}

gpointer start_watching()
{
    int s_fd;
    int i;
    struct sockaddr_in s_addr;
    
    s_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(s_fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }

    for(i=0;i<MAX_HAMTERS;i++)
    {
        g_hms[i].fd = -1;
    }
    
    memset((void *)&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(d_my_ipaddr);
    s_addr.sin_port = htons(BASE_PORT);
    
    if(bind(s_fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        printf("Bind failed\n");
        return NULL;
    }
    
    if(listen(s_fd, SOMAXCONN) < 0)
    {
        printf("Listen failed\n");
        return NULL;
    }
    
    for(;;)
    {
        memset(&g_hms[g_hm_id].c_addr, 0, sizeof(g_hms[g_hm_id].c_addr));
        socklen_t len = sizeof(g_hms[g_hm_id].c_addr);
        g_hms[g_hm_id].fd = accept(s_fd, (struct sockaddr *)&g_hms[g_hm_id].c_addr, &len);
        if(g_hms[g_hm_id].fd < 0)
        {
            printf("Accept failed\n");
        }
        else
        {
            if(pthread_create(&hm_thread[g_hm_id], NULL, hm_func, &g_hms[g_hm_id].fd) < 0)
            {
                printf("pthread_create failed\n");
                close(g_hms[g_hm_id].fd);
                g_hms[g_hm_id].fd = -1;
                close(s_fd);
                return NULL;
            }
	        g_hm_id++;
        }
    }
    
    for(i=0;i<g_hm_id;i++)
        pthread_join(hm_thread[i], 0);

    close(s_fd);
    
    return NULL;
}

