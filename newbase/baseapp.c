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

#include "pub.h"
#include "common.h"

#include "watcher.h"
#include "mcast.h"
#include "cmd_util.h"

GThread *sw_thread = NULL;
GThread *mc_thread = NULL;

GtkTextBuffer *log_buf;

GtkWidget *connect_tb;
GtkWidget *disconnect_tb;

GtkRadioButton *rb_left_forward;
GtkRadioButton *rb_left_stop;
GtkRadioButton *rb_left_backward;
GtkRadioButton *rb_right_forward;
GtkRadioButton *rb_right_stop;
GtkRadioButton *rb_right_backward;


/*
Forward
Stop

*/

void BASE_LOG(char *buffer)
{
//    GtkTextIter iter;
//    gtk_text_buffer_get_end_iter(log_buf, &iter);
//    gtk_text_buffer_insert(log_buf, &iter, buffer, -1);
g_printf("%s", buffer);
}


char         *s_my_ipaddr;
unsigned int d_my_ipaddr;

static void get_my_own_addr()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    s_my_ipaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    d_my_ipaddr = inet_network(s_my_ipaddr);

    BASE_LOG("My IP Address:");
    BASE_LOG(s_my_ipaddr);
    BASE_LOG("\n");
    
    close(fd);
}

void safe_exit()
{
    close_mc_fd();
    close_sw_fd();
    gtk_main_quit ();
}

void on_m_window_destroy (GtkWidget *window, gpointer data)
{
    safe_exit();
}

void on_toolbutton_exit_clicked (GtkWidget *button, gpointer data)
{
    safe_exit();
}

void on_toolbutton_connect_clicked (GtkWidget *button, gpointer data)
{
    BASE_LOG("connect clicked!\n");
    sw_thread = g_thread_new("Watching", start_watching, NULL);
    mc_thread = g_thread_new("Guard", multicast_guard, NULL);

    gtk_widget_set_sensitive(connect_tb, FALSE);    
    gtk_widget_set_sensitive(disconnect_tb, TRUE);
}

void on_toolbutton_disconnect_clicked (GtkWidget *button, gpointer data)
{
    gtk_widget_set_sensitive(connect_tb, TRUE);    
    gtk_widget_set_sensitive(disconnect_tb, FALSE);
}

void on_button_gc_send_clicked(GtkWidget *button, gpointer data)
{
    send_gc_command();
}

void on_button_light_on_clicked(GtkWidget *button, gpointer data)
{
    send_light_command(BA_LIGHT_ON);
}

void on_button_light_off_clicked(GtkWidget *button, gpointer data)
{
    send_light_command(BA_LIGHT_OFF);
}

void on_button_motion_cmd_clicked(GtkWidget *button, gpointer data)
{
    struct s_base_motion motion;

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_forward))) {
        motion.left_action = START_ACTION;
        motion.left_dir = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_stop))) {
        motion.left_action = STOP_ACTION;
        motion.left_dir = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_backward))) {
        motion.left_action = START_ACTION;
        motion.left_dir = NEGATIVE_DIR;
    }

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_forward))) {
        motion.right_action = START_ACTION;
        motion.right_dir = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_stop))){
        motion.right_action = STOP_ACTION;
        motion.right_dir = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_backward))){
        motion.right_action = START_ACTION;
        motion.right_dir = NEGATIVE_DIR;
    }

    send_motion_command(&motion);
}

void on_button_test_clicked(GtkWidget *button, gpointer data)
{
    send_test_command();
}

void on_button_servo_clicked(GtkWidget *button, gpointer data)
{
    send_servo_command();
}

void on_button_duty_incr_clicked(GtkWidget *button, gpointer data)
{
    send_pwm_duty_command(0);
}

void on_button_duty_dec_clicked(GtkWidget *button, gpointer data)
{
    send_pwm_duty_command(1);
}

void on_button_freq_incr_clicked(GtkWidget *button, gpointer data)
{
    send_pwm_freq_command(0);
}

void on_button_freq_dec_clicked(GtkWidget *button, gpointer data)
{
    send_pwm_freq_command(1);
}

int main (int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget  *window;

    GtkWidget  *log_tv;
        
    gtk_init (&argc, &argv);
        
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "newbase.ui", NULL);
    gtk_builder_connect_signals (builder, NULL);          

    window = GTK_WIDGET (gtk_builder_get_object (builder, "m_window"));

    connect_tb = GTK_WIDGET (gtk_builder_get_object (builder, "toolbutton_connect"));
    disconnect_tb = GTK_WIDGET (gtk_builder_get_object (builder, "toolbutton_disconnect"));

    rb_left_forward = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_left_forward"));
    rb_left_stop = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_left_stop"));
    rb_left_backward = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_left_reverse"));
    gtk_radio_button_join_group(rb_left_stop, rb_left_forward);
    gtk_radio_button_join_group(rb_left_backward, rb_left_forward);

    rb_right_forward = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_right_forward"));
    rb_right_stop = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_right_stop"));
    rb_right_backward = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "rb_right_reverse"));
    gtk_radio_button_join_group(rb_right_stop, rb_right_forward);
    gtk_radio_button_join_group(rb_right_backward, rb_right_forward);

    log_tv = GTK_WIDGET (gtk_builder_get_object (builder, "log_tv"));

    log_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_tv));
    gtk_text_buffer_set_text (log_buf, "Welcome to hamster base!\n", -1);

    g_object_unref (G_OBJECT (builder));
    
    gtk_widget_show_all(window);
    gtk_window_maximize(GTK_WINDOW(window));

    get_my_own_addr();

    gtk_main ();
        
    return 0;
}
