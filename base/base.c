#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "map.h"
#include "watcher.h"

GtkTextBuffer *log_buf;

static void socket_connect();

#define BASE_PORT  8888

GtkWidget* create_toolbar();
static void create_layout();

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
    gtk_text_buffer_set_text (log_buf, "Your 1st GtkTextView widget!", -1);
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
    GtkToolItem *cut_tb, *copy_tb, *paste_tb, *separator;
    GtkToolItem *exit_tb, *connect_tb, *disconnect_tb;
    GtkToolItem *hlp_tb;
    
    toolbar = gtk_toolbar_new();
    connect_tb = gtk_tool_button_new_from_stock (GTK_STOCK_CONNECT);
    g_signal_connect(G_OBJECT(connect_tb), "clicked", G_CALLBACK(socket_connect), NULL); 
    disconnect_tb = gtk_tool_button_new_from_stock (GTK_STOCK_DISCONNECT);
    g_signal_connect(G_OBJECT(disconnect_tb), "clicked", G_CALLBACK(socket_connect), NULL); 
    gtk_widget_set_sensitive(GTK_WIDGET(disconnect_tb), FALSE);
    cut_tb = gtk_tool_button_new_from_stock (GTK_STOCK_CUT);
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
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), cut_tb,         -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), copy_tb,        -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), paste_tb,       -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separator,      -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), hlp_tb,         -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), exit_tb,        -1);
    
    return (toolbar);
}

static void socket_connect()
{
    GThread *comm;
    comm = g_thread_create(start_watching, NULL, TRUE, NULL);
}

gpointer multicast_guard()
{
#if 0
    int s_fd, c_fd;
    struct sockaddr_in c_addr, s_addr;
    
    s_fd = socket(PF_INET, SOCK_DGRAM, 0);
    
    if(s_fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }

    unsigned char share = 1; // we will need to pass the address of this value
    if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &share, sizeof(share)) < 0) 
    {
        printf("setsockopt(allow multiple socket use");
        return NULL;
    }
    
    memset((void *)&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr("224.0.0.1");
    s_addr.sin_port = htons(BASE_PORT);
    
    if(bind(s_fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        printf("Bind failed\n");
        return NULL;
    }

    if (setsockopt(s_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &share, sizeof(share)) < 0) 
    {
        printf("setsockopt(multicast loop on)");
        return NULL;
    }

    command.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
    command.imr_interface.s_addr = htonl(INADDR_ANY);
    if (command.imr_multiaddr.s_addr == -1) {
        printf("Error: group of 224.0.0.1 not a legal multicast address\n");
        return NULL;
    }

    if (setsockopt(s_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &command, sizeof(command)) < 0) 
    {
        printf("Error in setsocket(add membership)");
        return NULL;
    }
#endif
    return NULL;
}

int main(int argc, char* argv[])
{
    if (!g_thread_supported())
        g_thread_init(NULL);
    
    gdk_threads_init();

    gdk_threads_enter();  
    gtk_init(&argc, &argv);
    
    create_layout();
    
    gtk_main ();
    gdk_threads_leave ();
    return 0;
}
