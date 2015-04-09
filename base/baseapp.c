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

struct {
  cairo_surface_t *image;  
} glob;

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

GtkRadioButton *radio_auto;
GtkRadioButton *radio_manual;

GtkGrid *grid_control;

GtkScale *left_speed;
GtkScale *right_speed;

GtkWidget  *m_win;

unsigned char *video_frame = NULL;

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

void on_radio_auto_toggled(GtkWidget *radio, gpointer data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        send_gc_command(AUTO_MODE);
    else
        send_gc_command(MANUAL_MODE);
}

void on_rb_left_forward_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.left_action = START_ACTION;
    motion.left_data = POSITIVE_DIR;
    motion.right_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_rb_left_stop_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.left_action = STOP_ACTION;
    motion.right_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_rb_left_reverse_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.left_action = START_ACTION;
    motion.left_data = NEGATIVE_DIR;
    motion.right_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_rb_right_forward_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.right_action = START_ACTION;
    motion.right_data = POSITIVE_DIR;
    motion.left_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_rb_right_stop_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.right_action = STOP_ACTION;
    motion.left_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_rb_right_reverse_toggled(GtkWidget *radio, gpointer data)
{
    struct s_base_motion motion;

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    motion.right_action = START_ACTION;
    motion.right_data = NEGATIVE_DIR;
    motion.left_action = NULL_ACTION;

    send_motion_command(&motion);
}

void on_button_motion_cmd_clicked(GtkWidget *button, gpointer data)
{
    struct s_base_motion motion;

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_forward))) {
        motion.left_action = START_ACTION;
        motion.left_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_stop))) {
        motion.left_action = STOP_ACTION;
        motion.left_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_backward))) {
        motion.left_action = START_ACTION;
        motion.left_data = NEGATIVE_DIR;
    }

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_forward))) {
        motion.right_action = START_ACTION;
        motion.right_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_stop))){
        motion.right_action = STOP_ACTION;
        motion.right_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_backward))){
        motion.right_action = START_ACTION;
        motion.right_data = NEGATIVE_DIR;
    }

    send_motion_command(&motion);
}

void right_speed_value_changed_cb(GtkWidget *button, gpointer data)
{
    struct s_base_motion motion;
    
    motion.right_action = SET_DUTY_ACTION;
    motion.right_data = (int)gtk_range_get_value (GTK_RANGE(right_speed));

    motion.left_action = NULL_ACTION;
    
    send_motion_command(&motion);
}

void left_speed_value_changed_cb(GtkWidget *button, gpointer data)
{
    struct s_base_motion motion;

    motion.left_action = SET_DUTY_ACTION;
    motion.left_data = (int)gtk_range_get_value (GTK_RANGE(left_speed));
    
    motion.right_action = NULL_ACTION;
    
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

void on_led_switch_button_press_event (GtkWidget *sw, gpointer data)
{
    if(gtk_switch_get_active (GTK_SWITCH(sw)))
        send_light_command(BA_LIGHT_ON);
    else
        send_light_command(BA_LIGHT_OFF);
}

static void do_drawing(cairo_t *cr)
{
    cairo_set_source_surface(cr, glob.image, 10, 10);
    cairo_paint(cr);    
}

void on_hm_sight_draw(GtkWidget *sight, cairo_t *cr, gpointer data)
{
    do_drawing(cr);
}

int tms=0;
static gboolean time_handler(GtkWidget *widget)
{
if((tms&1)==0)
glob.image = cairo_image_surface_create_from_png("mask1.png");
else
glob.image = cairo_image_surface_create_from_png("mask2.png");

tms++;

  gtk_widget_queue_draw(widget);

  return TRUE;
}

void update_video(unsigned char *data, int width, int height)
{
    glob.image = cairo_image_surface_create_for_data(data, 
                               CAIRO_FORMAT_RGB24, width, height, 
                               cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width));
    
    gtk_widget_queue_draw(m_win);
}


int main (int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget  *log_tv;

    glob.image = cairo_image_surface_create_from_png("mask1.png");

    video_frame = malloc(640*480*10);
        
    gtk_init (&argc, &argv);
        
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "newbase.ui", NULL);
    gtk_builder_connect_signals (builder, NULL);          

    m_win = GTK_WIDGET (gtk_builder_get_object (builder, "m_window"));

    connect_tb = GTK_WIDGET (gtk_builder_get_object (builder, "toolbutton_connect"));
    disconnect_tb = GTK_WIDGET (gtk_builder_get_object (builder, "toolbutton_disconnect"));

    grid_control = GTK_GRID (gtk_builder_get_object (builder, "grid_control"));

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

    radio_auto = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "radio_auto"));
    radio_manual = GTK_RADIO_BUTTON (gtk_builder_get_object (builder, "radio_manual"));

    left_speed = GTK_SCALE (gtk_builder_get_object (builder, "left_speed"));
    right_speed = GTK_SCALE (gtk_builder_get_object (builder, "right_speed"));

    log_tv = GTK_WIDGET (gtk_builder_get_object (builder, "log_tv"));

    log_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_tv));
    gtk_text_buffer_set_text (log_buf, "Welcome to hamster base!\n", -1);

    g_object_unref (G_OBJECT (builder));
 
    //g_timeout_add(500, (GSourceFunc) time_handler, (gpointer) m_win);
   
    gtk_widget_show_all(m_win);
    gtk_window_maximize(GTK_WINDOW(m_win));

    get_my_own_addr();

    gtk_main ();
        
    cairo_surface_destroy(glob.image);

    return 0;
}
