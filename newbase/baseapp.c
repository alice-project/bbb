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
#include "img_xpm.h"
#include "odometer.h"

static GtkWidget *m_win = NULL;
GdkRGBA rgba_color;
GtkWidget *image_status;
GdkPixbuf *pixbuf_status;

guint g_status = 0;

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


void set_light1_red()
{

}

void set_light1_yellow()
{

}

void set_light1_green()
{

}

void set_ssonic1_dist(int d)
{

}



static base_set_win_style(GtkWidget *s_win)
{
    gtk_container_set_border_width (GTK_CONTAINER (s_win), 0);
    gtk_window_set_title (GTK_WINDOW (s_win), "BASE");
    gtk_window_set_default_size (GTK_WINDOW (s_win), 200, 600);
}

static void do_drawing(cairo_t *cr, GtkWidget *widget)
{
    cairo_surface_t *image;
    int width, height;
    
    image = cairo_image_surface_create_from_png("./img1.png");
    gtk_window_get_size(GTK_WINDOW(widget), &width, &height);
    cairo_set_source_surface(cr, image, width, height);
    cairo_paint(cr);
}

static gboolean on_draw_button(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{  
    do_drawing(cr, widget);  
    return FALSE;
}

static gboolean update_status(gpointer data)
{
    GdkPixbuf *pixbuf_img;

    if(g_status==0)    
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(circle1_xpm);
    else if(g_status==1)    
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(circle2_xpm);
    else if(g_status==2)    
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(circle3_xpm);
    else
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(circle4_xpm);

    gtk_image_set_from_pixbuf(GTK_IMAGE(data), pixbuf_img);

    g_status = (g_status+1)&0x3;

    return TRUE;
}

gboolean camera_win_created=FALSE;
GtkWidget *camera_win=NULL;
static void create_camera_win(GtkWidget* pWindow)
{
    if(camera_win_created)  return;

    GdkPixbuf *pixbuf_img;

    GtkWidget *img;

    camera_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img14_xpm);
    img = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_container_add (GTK_CONTAINER (camera_win), img);
    gtk_window_set_title (GTK_WINDOW (camera_win), "Pop Up window");
    gtk_container_set_border_width (GTK_CONTAINER (camera_win), 10);
    gtk_window_set_resizable (GTK_WINDOW (camera_win), FALSE);
    gtk_window_set_decorated (GTK_WINDOW (camera_win), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (camera_win), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (camera_win), TRUE);
    gtk_widget_set_size_request (camera_win, 150, 150);
    gtk_window_set_transient_for (GTK_WINDOW (camera_win), GTK_WINDOW (pWindow));
//    gtk_window_set_position (GTK_WINDOW (camera_win), GTK_WIN_POS_CENTER);
    gtk_window_move (GTK_WINDOW (camera_win), gdk_screen_width(), gdk_screen_height());

    gtk_widget_set_events (camera_win, GDK_FOCUS_CHANGE_MASK);

    GdkColor color;
    gdk_color_parse ("#3b3131", &color);
    gtk_widget_override_background_color(GTK_WIDGET (camera_win), GTK_STATE_NORMAL, &rgba_color);

    gtk_widget_show_all (camera_win);
    gtk_widget_grab_focus (camera_win);
}

static void control_camera_win(GtkButton* button, GtkWidget* pWindow)
{
    if(!camera_win_created)
    {
        create_camera_win(pWindow);
        camera_win_created = TRUE;
    }
    else
    {
        gtk_widget_destroy(camera_win);
        camera_win_created = FALSE;
    }
}

GtkWidget *create_left_box()
{
    GtkWidget *button_box;
    GtkWidget *button_quadmotor;
    GtkWidget *button_plane;
    GtkWidget *button_robot;
    GtkWidget *button_camera;
    GtkWidget *button_setting;
    GtkWidget *button_exit;
    GtkWidget *darea;
    GdkPixbuf *pixbuf_img;

    GtkWidget *img_quadmotor, *img_plane, *img_robot, *img_camera, *img_setting, *img_exit;
    button_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    button_quadmotor = gtk_button_new();
    gtk_box_pack_start (GTK_BOX (button_box), button_quadmotor, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img10_xpm);
    img_quadmotor = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_quadmotor), img_quadmotor);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_plane = gtk_button_new();
    gtk_box_pack_start (GTK_BOX (button_box), button_plane, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img8_xpm);
    img_plane = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_plane), img_plane);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_robot = gtk_button_new();
    gtk_box_pack_start (GTK_BOX (button_box), button_robot, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img7_xpm);
    img_robot = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_robot), img_robot);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_camera = gtk_button_new();
    g_signal_connect (G_OBJECT (button_camera),
                    "clicked",
                    G_CALLBACK (control_camera_win),
                    (gpointer) m_win);

    gtk_box_pack_start (GTK_BOX (button_box), button_camera, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img13_xpm);
    img_camera = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_camera), img_camera);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_setting = gtk_button_new();
    gtk_box_pack_start (GTK_BOX (button_box), button_setting, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img3_xpm);
    img_setting = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_setting), img_setting);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_exit = gtk_button_new();
    gtk_box_pack_start (GTK_BOX (button_box), button_exit, TRUE, FALSE, 0);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img4_xpm);
    img_exit = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_exit), img_exit);
    g_object_unref(G_OBJECT(pixbuf_img));
    
    g_signal_connect (button_exit, "clicked", G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(button_exit), 0);

    gtk_container_set_border_width (GTK_CONTAINER(button_box), 0);

    return button_box;
}

GtkWidget *create_central_box()
{
    GtkWidget *mid_box;
    GtkWidget *top_status;
    GtkWidget *pane;
    GtkWidget *frame;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;
    GtkWidget *odometer;
    GtkWidget *image;

    GdkPixbuf *pixbuf_img;
    GtkWidget *img_status;

    GtkWidget *button_box;

    mid_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    top_status = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    button2 = gtk_button_new_with_label ("status");
    button3 = gtk_button_new_with_label ("status");

    pixbuf_img = gdk_pixbuf_new_from_xpm_data(circle1_xpm);
    image_status = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_box_pack_start (GTK_BOX (top_status), image_status, TRUE, FALSE, 0);
    g_timeout_add(500, (GSourceFunc)update_status, (gpointer)image_status);
    g_object_unref(G_OBJECT(pixbuf_img));

    pixbuf_img = gdk_pixbuf_new_from_xpm_data(dot1_xpm);
    image = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_box_pack_start (GTK_BOX (top_status), image, TRUE, FALSE, 0);
    g_object_unref(G_OBJECT(pixbuf_img));
    
    gtk_box_pack_start (GTK_BOX (top_status), button2, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (top_status), button3, TRUE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (mid_box), top_status, FALSE, FALSE, 10);

    pane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start (GTK_BOX (mid_box), pane, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_widget_set_size_request (frame, 600, 600);
    gtk_paned_pack1(GTK_PANED(pane), frame, TRUE, FALSE);

    pixbuf_img = gdk_pixbuf_new_from_xpm_data(earth_xpm);
    image = gtk_image_new_from_pixbuf (pixbuf_img);
    gtk_container_add (GTK_CONTAINER (frame), image);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);

    button1 = gtk_button_new_with_label ("Hi1 there");
    button2 = gtk_button_new_with_label ("Hi2 there");
    button3 = gtk_button_new_with_label ("Hi3 there");
    odometer = odometer_new ();
    gtk_box_pack_start (GTK_BOX (button_box), button1, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (button_box), button2, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (button_box), button3, TRUE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (button_box), odometer, TRUE, TRUE, 0);
    gtk_paned_pack2(GTK_PANED(pane), button_box, TRUE, FALSE);

    return mid_box;
}

GtkWidget *create_right_box()
{
    GtkWidget *button_box;
    GtkWidget *button1;

    button_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);

    button1 = gtk_button_new_with_label ("Hi-there");
    gtk_container_add (GTK_CONTAINER(button_box), button1);
    gtk_container_set_border_width (GTK_CONTAINER(button_box), 10);

    return button_box;
}

int main (int argc, char *argv[])
{
    GtkWidget *m_box;
    GtkWidget *left_box;
    GtkWidget *mid_box;
    GtkWidget *right_box;    

    gtk_init (&argc, &argv);

    if(m_win == NULL)
    {
#if 0
m_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        m_box = odometer_new ();
        gtk_container_add (GTK_CONTAINER (m_win), m_box);
gtk_window_set_default_size(GTK_WINDOW(m_win),400, 400);
        g_signal_connect (m_win, "destroy", G_CALLBACK (gtk_main_quit), &m_win);

#else
        m_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gdk_rgba_parse(&rgba_color, "#aaaaaa");
        gtk_widget_override_background_color(m_win, GTK_STATE_NORMAL, &rgba_color);

        g_signal_connect (m_win, "destroy", G_CALLBACK (gtk_main_quit), &m_win);

        base_set_win_style(m_win);

        m_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add (GTK_CONTAINER (m_win), m_box);

        left_box = create_left_box();
        gtk_box_pack_start (GTK_BOX (m_box), left_box, FALSE, FALSE, 5);

        mid_box = create_central_box();
        gtk_box_pack_start (GTK_BOX (m_box), mid_box, TRUE, FALSE, 0);

        right_box = create_right_box();
        gtk_box_pack_start (GTK_BOX (m_box), right_box, FALSE, FALSE, 0);
#endif

        if (!gtk_widget_get_visible (m_win))
        {
            gtk_widget_show_all (m_win);
//            gtk_window_maximize(GTK_WINDOW(m_win));
        }
        else
        {
            gtk_widget_destroy (m_win);
            m_win = NULL;
        }

        gtk_main ();
    }

    return 0;
}

