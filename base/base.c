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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "pub.h"
#include "common.h"
#include "map.h"
#include "watcher.h"

extern int g_hm_id;
extern struct s_hm g_hms[MAX_HAMTERS];

char         *s_my_ipaddr;
unsigned int d_my_ipaddr;

GtkTextBuffer *log_buf;

static void socket_connect();
static void send_commands();

GtkWidget* create_toolbar();
static void create_layout();

static inline void BASE_LOG(char *buffer)
{
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(log_buf, &iter);
    gtk_text_buffer_insert(log_buf, &iter, buffer, -1);
}

static void get_my_addr()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "eth1", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    s_my_ipaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    d_my_ipaddr = inet_network(s_my_ipaddr);

    BASE_LOG("My IP Address:");
    BASE_LOG(s_my_ipaddr);
    BASE_LOG("\n");
    
    close(fd);
}

static void create_layout()
{
    GtkWidget *window;
    GtkWidget *status_bar;
    GtkWidget *tool_bar;
    GtkWidget *top_box, *mid_pane;
    GtkWidget *text_view, *scrolled_win;
    GtkWidget *map;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title (GTK_WINDOW (window), "Tracking System");
    gtk_container_set_border_width (GTK_CONTAINER (window), 1);
    gtk_widget_set_size_request (window, gdk_screen_width(), gdk_screen_height());
    
    top_box = gtk_vbox_new (FALSE, 1);

    tool_bar = create_toolbar();
    gtk_box_pack_start(GTK_BOX(top_box), tool_bar, FALSE, TRUE, 0);

    mid_pane = gtk_hpaned_new ();
    text_view = gtk_text_view_new ();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    log_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_buffer_set_text (log_buf, "Welcome to hamster base!\n", -1);
    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrolled_win), text_view);
    
    map = ants_map_new();
    gtk_widget_set_size_request(map, 600, 400);
    gtk_paned_pack1(GTK_PANED (mid_pane), scrolled_win, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED (mid_pane), map, TRUE, TRUE);
    
    gtk_box_pack_start (GTK_BOX (top_box), mid_pane, TRUE, TRUE, 0);
    
    g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    status_bar = gtk_statusbar_new();
    gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "tracking");
    gtk_box_pack_start (GTK_BOX (top_box), status_bar, FALSE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (window), top_box);
    
    gtk_widget_show_all (window);
    gdk_window_maximize(window->window);
}


GtkWidget* create_toolbar()
{
    GtkWidget* toolbar;
    GtkToolItem *cmd_tb, *copy_tb, *paste_tb, *separator;
    GtkToolItem *exit_tb, *connect_tb, *disconnect_tb;
    GtkToolItem *hlp_tb;
    
    toolbar = gtk_toolbar_new();
    connect_tb = gtk_tool_button_new_from_stock (GTK_STOCK_CONNECT);
    g_signal_connect(G_OBJECT(connect_tb), "clicked", G_CALLBACK(socket_connect), NULL); 
    disconnect_tb = gtk_tool_button_new_from_stock (GTK_STOCK_DISCONNECT);
    g_signal_connect(G_OBJECT(disconnect_tb), "clicked", G_CALLBACK(socket_connect), NULL); 
    gtk_widget_set_sensitive(GTK_WIDGET(disconnect_tb), FALSE);
    cmd_tb = gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
    g_signal_connect(G_OBJECT(cmd_tb), "clicked", G_CALLBACK(send_commands), NULL); 
    copy_tb = gtk_tool_button_new_from_stock (GTK_STOCK_COPY);
    paste_tb = gtk_tool_button_new_from_stock (GTK_STOCK_PASTE);
    hlp_tb = gtk_tool_button_new_from_stock (GTK_STOCK_HELP);
    
    exit_tb = gtk_tool_button_new_from_stock (GTK_STOCK_QUIT);
    g_signal_connect(G_OBJECT(exit_tb), "clicked", G_CALLBACK(gtk_main_quit), NULL); 
    
    separator = gtk_separator_tool_item_new ();
    
    gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);
    
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), connect_tb,      0);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), disconnect_tb,  -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), cmd_tb,         -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), copy_tb,        -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), paste_tb,       -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separator,      -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), hlp_tb,         -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), exit_tb,        -1);
    
    return (toolbar);
}

gpointer multicast_guard()
{
    struct sockaddr_in remote_addr;
    struct in_addr ia;
    int sockfd;
    unsigned char rcv_buf[BUFLEN];
    unsigned char snd_buf[BUFLEN];
    unsigned int socklen, n;
    struct hostent *m_group;
    struct ip_mreqn m_req;
    struct s_com *rcv_msg, *snd_msg;
    struct s_mc_ipaddr *payload;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        BASE_LOG("socket creating err in udptalk\n");
        return NULL;
    }
    
    memset(&m_req, 0, sizeof(struct ip_mreqn));
    if ((m_group = gethostbyname(MUL_IPADDR)) == NULL) 
    {
        BASE_LOG("gethostbyname\n");
        return NULL;
    }
    
    bcopy((void *)m_group->h_addr, &ia, m_group->h_length);
    bcopy(&ia, &m_req.imr_multiaddr.s_addr, sizeof(struct in_addr));
    
    m_req.imr_address.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_req, sizeof(struct ip_mreqn)) == -1) 
    {
        BASE_LOG("setsockopt\n");
        return NULL;
    }
    
    socklen = sizeof(struct sockaddr_in);
    memset(&remote_addr, 0, socklen);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(BASE_PORT);

    if (inet_pton(AF_INET, MUL_IPADDR, &remote_addr.sin_addr) <= 0) 
    {
        BASE_LOG("Wrong dest IP address!\n");
        return NULL;
    }
    
    if (bind(sockfd, (struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in)) == -1) 
    {
        BASE_LOG("Bind error\n");
        return NULL;
    }
    
    BASE_LOG("BASE is ready!\n");
    for(;;) 
    {
        n = recvfrom(sockfd, rcv_buf, BUFLEN, 0, (struct sockaddr *) &remote_addr, &socklen);
        if (n < sizeof(struct s_com)) {
            continue;
        } 
        else 
        {
            BASE_LOG("NEW ANT IS COMMING\n");
            memset(snd_buf, 0, sizeof(snd_buf));
            snd_msg = (struct s_com *)snd_buf;
            rcv_msg = (struct s_com *)rcv_buf;

            switch(rcv_msg->code)
            {
                case HM_REQUEST_BASE:
                {
                    snd_msg->code = BA_MC_IPADDR;
                    snd_msg->len = sizeof(struct s_mc_ipaddr);
                    payload = (struct s_mc_ipaddr *)(snd_msg + sizeof(struct s_com));
                    payload->ipaddr = htonl(d_my_ipaddr);
                    n = sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));

                    break;
                }
                default:
                    break;
            }
        }
        sleep(10);
    }

    return NULL;
}

static void socket_connect()
{
    GThread *comm, *mc_thread;
    comm = g_thread_create(start_watching, NULL, TRUE, NULL);
    mc_thread = g_thread_create(multicast_guard, NULL, TRUE, NULL);
}

static void send_commands()
{
    int n;
    char snd_buf[BUFLEN];
    struct s_com *msg = (struct s_com *)snd_buf;

    if(g_hm_id < 1)    
    {
        printf("invalid fd\n");
        return;
    }

    if(g_hms[g_hm_id-1].fd < 0)
    {
        printf("invalid fd\n");
        return;
    }

printf("sending to %d\n", g_hms[g_hm_id-1].fd);

    memset(snd_buf, 0, sizeof(snd_buf));
    msg->code = BA_INSTRUCT_MOVE;

    n = sendto(g_hms[g_hm_id-1].fd, snd_buf, BUFLEN, 0, (struct sockaddr *) &g_hms[MAX_HAMTERS].c_addr, sizeof(struct sockaddr_in));

}

int main(int argc, char* argv[])
{
    if (!g_thread_supported())
        g_thread_init(NULL);
    
    gdk_threads_init();

    gdk_threads_enter();  
    gtk_init(&argc, &argv);
    
    create_layout();
    
    get_my_addr();

    gtk_main ();
    gdk_threads_leave ();
    return 0;
}
