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

#define M_PI  3.14

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

GtkScale *left_speed, *right_speed;
GtkScale *servo1_angle, *servo2_angle;

GtkWidget  *m_win;

GtkWidget *ssonic1_light1, *ssonic1_light2, *ssonic1_light3;
GtkWidget *ssonic2_light1, *ssonic2_light2, *ssonic2_light3;
GtkLabel *ssonic1_dist, *ssonic2_dist;

GtkDrawingArea *servo1_clock;
GtkDrawingArea *servo2_clock;

GtkEntry *entry_rawtext_cmd;

struct s_base_motion r_motion;

/*
 light: 0=GRAY(not active), 1=GREEN, 2=YELLOW, 3=RED
 */
enum TRAFFIC_LIGHT {
    TRAFFIC_LIGHT_GREEN=1,
    TRAFFIC_LIGHT_YELLOW,
    TRAFFIC_LIGHT_RED,
};
static int ssonic1_light=0;
static int ssonic2_light=0;

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
#ifdef DEBUG
g_printf("%s", buffer);
#endif
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

void on_btn_send_rawcmd_clicked (GtkWidget *button, gpointer data)
{
    g_printf("text:%s\n", gtk_entry_get_text(entry_rawtext_cmd));
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

    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.left_action = START_ACTION;
    r_motion.left_data = POSITIVE_DIR;

    send_motion_command(&r_motion);
}

void on_rb_left_stop_toggled(GtkWidget *radio, gpointer data)
{
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.left_action = STOP_ACTION;

    send_motion_command(&r_motion);
}

void on_rb_left_reverse_toggled(GtkWidget *radio, gpointer data)
{
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.left_action = START_ACTION;
    r_motion.left_data = NEGATIVE_DIR;

    send_motion_command(&r_motion);
}

void on_rb_right_forward_toggled(GtkWidget *radio, gpointer data)
{
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.right_action = START_ACTION;
    r_motion.right_data = POSITIVE_DIR;

    send_motion_command(&r_motion);
}

void on_rb_right_stop_toggled(GtkWidget *radio, gpointer data)
{
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.right_action = STOP_ACTION;

    send_motion_command(&r_motion);
}

void on_rb_right_reverse_toggled(GtkWidget *radio, gpointer data)
{
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
        return;

    r_motion.right_action = START_ACTION;
    r_motion.right_data = NEGATIVE_DIR;

    send_motion_command(&r_motion);
}

void on_button_motion_cmd_clicked(GtkWidget *button, gpointer data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_forward))) {
        r_motion.left_action = START_ACTION;
        r_motion.left_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_stop))) {
        r_motion.left_action = STOP_ACTION;
        r_motion.left_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_left_backward))) {
        r_motion.left_action = START_ACTION;
        r_motion.left_data = NEGATIVE_DIR;
    }

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_forward))) {
        r_motion.right_action = START_ACTION;
        r_motion.right_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_stop))){
        r_motion.right_action = STOP_ACTION;
        r_motion.right_data = POSITIVE_DIR;
    } else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_right_backward))){
        r_motion.right_action = START_ACTION;
        r_motion.right_data = NEGATIVE_DIR;
    }

    send_motion_command(&r_motion);
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

void servo1_angle_value_changed_cb(GtkWidget *button, gpointer data)
{
    send_servo_command(0, (int)gtk_range_get_value (GTK_RANGE(servo1_angle)));
}

void servo2_angle_value_changed_cb(GtkWidget *button, gpointer data)
{
    send_servo_command(1, (int)gtk_range_get_value (GTK_RANGE(servo2_angle)));
}

void on_led_switch_button_press_event (GtkWidget *sw, gpointer data)
{
    if(gtk_switch_get_active (GTK_SWITCH(sw)))
        send_light_command(BA_LIGHT_ON);
    else
        send_light_command(BA_LIGHT_OFF);
}

void on_camera_switch_button_press_event (GtkWidget *sw, gpointer data)
{
    if(gtk_switch_get_active (GTK_SWITCH(sw)))
        send_camera_command(1);
    else
        send_camera_command(0);
}

static gboolean on_servo1_clock_button_press_event (GtkWidget      *widget,
                                             GdkEventButton *event,
                                             gpointer        data)
{
  if (event->button == 1)
    g_printf("%d,%d\n", event->x, event->y);

    return TRUE;
}

gboolean on_servo2_clock_button_press_event (GtkWidget      *widget,
                                             GdkEventButton *event,
                                             gpointer        data)
{
    if (event->button == 1)
        g_printf("%d,%d\n", event->x, event->y);

    return TRUE;
}

static void do_hm_sight_drawing(cairo_t *cr)
{
    cairo_set_source_surface(cr, glob.image, 10, 10);
    cairo_paint(cr);    
}

void on_hm_sight_draw(GtkWidget *sight, cairo_t *cr, gpointer data)
{
    do_hm_sight_drawing(cr);
}

static void do_basic_servo_clock_drawing(cairo_t *cr)
{
    cairo_set_line_width (cr, 2);

    cairo_new_sub_path (cr); 
    cairo_arc (cr, 50, 50, 40, 0, 2*M_PI);
    cairo_move_to (cr, 45, 50);
    cairo_rel_line_to(cr, 10, 0);
    cairo_move_to (cr, 50, 45);
    cairo_rel_line_to(cr, 0, 10);

    cairo_move_to (cr, 10, 50);
    cairo_rel_line_to(cr, 5, 0);
    cairo_move_to (cr, 90, 50);
    cairo_rel_line_to(cr, -5, 0);
    cairo_move_to (cr, 50, 10);
    cairo_rel_line_to(cr, 0, 5);
    cairo_move_to (cr, 50, 90);
    cairo_rel_line_to(cr, 0, -5);

    cairo_move_to(cr, 50, 50);
    cairo_rel_line_to (cr, 0, -40);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source_rgb (cr, 0, 0.8, 0); 
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0, 0, 0); 
    cairo_stroke (cr);
}


static void do_servo_clock_drawing(cairo_t *cr)
{
    cairo_set_line_width (cr, 2);

    cairo_new_sub_path (cr); 
    cairo_arc (cr, 50, 50, 40, 0, 2*M_PI);
    cairo_move_to (cr, 45, 50);
    cairo_rel_line_to(cr, 10, 0);
    cairo_move_to (cr, 50, 45);
    cairo_rel_line_to(cr, 0, 10);

    cairo_move_to (cr, 10, 50);
    cairo_rel_line_to(cr, 5, 0);
    cairo_move_to (cr, 90, 50);
    cairo_rel_line_to(cr, -5, 0);
    cairo_move_to (cr, 50, 10);
    cairo_rel_line_to(cr, 0, 5);
    cairo_move_to (cr, 50, 90);
    cairo_rel_line_to(cr, 0, -5);

    cairo_move_to(cr, 50, 50);
    cairo_rel_line_to (cr, 0, -40);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source_rgb (cr, 0, 0.8, 0); 
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0, 0, 0); 
    cairo_stroke (cr);
}

void on_servo1_clock_draw(GtkWidget *sight, cairo_t *cr, gpointer data)
{
    do_basic_servo_clock_drawing(cr);
}

void on_servo2_clock_draw(GtkWidget *sight, cairo_t *cr, gpointer data)
{
    do_basic_servo_clock_drawing(cr);
}

static void do_ssonic_light_drawing(cairo_t *cr, int light)
{
    cairo_pattern_t *linpat, *radpat;
    if(light == 1)  // green
    {
        linpat = cairo_pattern_create_linear (0, 0, 40, 40);
        cairo_pattern_add_color_stop_rgb (linpat, 0, 0, 1, 0);
        cairo_pattern_add_color_stop_rgb (linpat, 1, 0, 1, 0);

        radpat = cairo_pattern_create_radial (6, 6, 2.5, 10, 10, 14);
        cairo_pattern_add_color_stop_rgba (radpat, 0, 0, 1, 0, 1);
        cairo_pattern_add_color_stop_rgba (radpat, 0.6, 0, 1, 0, 0);
    } else if (light == 2) {  //yellow
        linpat = cairo_pattern_create_linear (0, 0, 20, 20);
        cairo_pattern_add_color_stop_rgb (linpat, 0, 1, 1, 0);
        cairo_pattern_add_color_stop_rgb (linpat, 1, 1, 1, 0);

        radpat = cairo_pattern_create_radial (6, 6, 2.5, 10, 10, 14);
        cairo_pattern_add_color_stop_rgba (radpat, 0, 1, 1, 0, 1);
        cairo_pattern_add_color_stop_rgba (radpat, 0.6, 1, 1, 0, 0);
    } else if (light == 3) {  // red
        linpat = cairo_pattern_create_linear (0, 0, 20, 20);
        cairo_pattern_add_color_stop_rgb (linpat, 0, 1, 0, 0);
        cairo_pattern_add_color_stop_rgb (linpat, 1, 1, 0, 0);

        radpat = cairo_pattern_create_radial (6, 6, 2.5, 10, 10, 14);
        cairo_pattern_add_color_stop_rgba (radpat, 0, 1, 0, 0, 1);
        cairo_pattern_add_color_stop_rgba (radpat, 0.6, 1, 0, 0, 0);
    } else {  // grey
        linpat = cairo_pattern_create_linear (0, 0, 20, 20);
        cairo_pattern_add_color_stop_rgb (linpat, 0.58, 0.64, 0.65, 1);
        cairo_pattern_add_color_stop_rgb (linpat, 0.58, 0.64, 0.65, 0);

        radpat = cairo_pattern_create_radial (6, 6, 2.5, 10, 10, 14);
        cairo_pattern_add_color_stop_rgba (radpat, 0, 0.58, 0.64, 0.65, 1);
        cairo_pattern_add_color_stop_rgba (radpat, 0.6, 0.58, 0.64, 0.65, 0);
    }
    
    cairo_set_source (cr, linpat);
    cairo_mask (cr, radpat);
    cairo_fill (cr);
}

void on_ssonic1_light1_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic1_light==TRAFFIC_LIGHT_GREEN)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_GREEN);
    else
        do_ssonic_light_drawing(cr, 0);
}

void on_ssonic1_light2_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic1_light==TRAFFIC_LIGHT_YELLOW)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_YELLOW);
    else
        do_ssonic_light_drawing(cr, 0);
}

void on_ssonic1_light3_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic1_light==TRAFFIC_LIGHT_RED)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_RED);
    else
        do_ssonic_light_drawing(cr, 0);
}

void on_ssonic2_light1_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic2_light==TRAFFIC_LIGHT_GREEN)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_GREEN);
    else
        do_ssonic_light_drawing(cr, 0);
}

void on_ssonic2_light2_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic2_light==TRAFFIC_LIGHT_YELLOW)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_YELLOW);
    else
        do_ssonic_light_drawing(cr, 0);
}

void on_ssonic2_light3_draw(GtkWidget *light, cairo_t *cr, gpointer data)
{
    if(ssonic2_light==TRAFFIC_LIGHT_GREEN)
        do_ssonic_light_drawing(cr, TRAFFIC_LIGHT_GREEN);
    else
        do_ssonic_light_drawing(cr, 0);
}

/*int tms=0;
static gboolean time_handler(GtkWidget *widget)
{
if((tms&1)==0)
glob.image = cairo_image_surface_create_from_png("mask1.png");
else
glob.image = cairo_image_surface_create_from_png("mask2.png");

tms++;

  gtk_widget_queue_draw(widget);

  return TRUE;
}*/

static gboolean time_handler(GtkWidget *widget)
{
    ssonic1_light++;
    ssonic2_light++;

    if(ssonic1_light>2)
        ssonic1_light = 0;
    if(ssonic2_light>2)
        ssonic2_light=0;

    gtk_widget_queue_draw(ssonic1_light1);
    gtk_widget_queue_draw(ssonic1_light2);
    gtk_widget_queue_draw(ssonic1_light3);
    gtk_widget_queue_draw(ssonic2_light1);
    gtk_widget_queue_draw(ssonic2_light2);
    gtk_widget_queue_draw(ssonic2_light3);

    return TRUE;
}

void set_light1_red()
{
    ssonic1_light = TRAFFIC_LIGHT_GREEN;
    gtk_widget_queue_draw(ssonic1_light1);
    gtk_widget_queue_draw(ssonic1_light2);
    gtk_widget_queue_draw(ssonic1_light3);
}

void set_light1_yellow()
{
    ssonic1_light = TRAFFIC_LIGHT_YELLOW;
    gtk_widget_queue_draw(ssonic1_light1);
    gtk_widget_queue_draw(ssonic1_light2);
    gtk_widget_queue_draw(ssonic1_light3);
}

void set_light1_green()
{
    ssonic1_light = TRAFFIC_LIGHT_GREEN;
    gtk_widget_queue_draw(ssonic1_light1);
    gtk_widget_queue_draw(ssonic1_light2);
    gtk_widget_queue_draw(ssonic1_light3);
}

void set_ssonic1_dist(int d)
{
    gchar dist[10];

    memset(dist, 0, sizeof(dist));
    if(d > 1000000)
        g_sprintf(dist, "D:infinite");
    else
        g_sprintf(dist, "D:%d",  d);

    gtk_label_set_text(ssonic1_dist, dist);
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

    ssonic1_light1 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic1_light1"));
    ssonic1_light2 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic1_light2"));
    ssonic1_light3 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic1_light3"));
    ssonic2_light1 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic2_light1"));
    ssonic2_light2 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic2_light2"));
    ssonic2_light3 = GTK_WIDGET (gtk_builder_get_object (builder, "ssonic2_light3"));

    ssonic1_dist = GTK_LABEL (gtk_builder_get_object (builder, "ssonic1_dist"));
    ssonic2_dist = GTK_LABEL (gtk_builder_get_object (builder, "ssonic2_dist"));

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

	r_motion.left_action = START_ACTION;
    r_motion.left_data = POSITIVE_DIR;
    r_motion.right_action = START_ACTION;
    r_motion.right_data = POSITIVE_DIR;

    servo1_angle = GTK_SCALE (gtk_builder_get_object (builder, "servo1_angle"));
    servo2_angle = GTK_SCALE (gtk_builder_get_object (builder, "servo1_angle"));

    entry_rawtext_cmd = GTK_ENTRY (gtk_builder_get_object (builder, "entry_rawtext_cmd"));

/*    servo1_clock = GTK_DRAWING_AREA (gtk_builder_get_object (builder, "servo1_clock"));
    servo2_clock = GTK_DRAWING_AREA (gtk_builder_get_object (builder, "servo2_clock"));
    gtk_widget_set_size_request (GTK_WIDGET(servo1_clock), 100, 100);
    g_signal_connect (servo1_clock, "button-press-event", G_CALLBACK (on_servo1_clock_button_press_event), NULL);
    g_signal_connect (servo2_clock, "button-press-event", G_CALLBACK (on_servo2_clock_button_press_event), NULL);
*/
    log_tv = GTK_WIDGET (gtk_builder_get_object (builder, "log_tv"));

    log_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_tv));
    gtk_text_buffer_set_text (log_buf, "Welcome to hamster base!\n", -1);

    g_object_unref (G_OBJECT (builder));
 
//    g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) m_win);
   
    gtk_widget_show_all(m_win);
    gtk_window_maximize(GTK_WINDOW(m_win));

    get_my_own_addr();

    gtk_main ();
        
    cairo_surface_destroy(glob.image);

    return 0;
}
