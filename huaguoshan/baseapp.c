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

#include "img_xpm.h"
#include "odometer.h"
#include "video.h"

static GtkWidget *m_win = NULL;
gboolean camera_win_created=FALSE;
GtkWidget *camera_win=NULL;

GdkRGBA rgba_color;
GtkWidget *image_status;
GdkPixbuf *pixbuf_status;
GtkWidget *image_map;

guint g_status = 0;

guint g_release_thrds=FALSE;

struct s_base_motion r_motion;

/*
 light: 0=GRAY(not active), 1=GREEN, 2=YELLOW, 3=RED
 */
enum TRAFFIC_LIGHT {
    TRAFFIC_LIGHT_GREEN=1,
    TRAFFIC_LIGHT_YELLOW,
    TRAFFIC_LIGHT_RED,
};

GThread *sw_thread = NULL;
GThread *mc_thread = NULL;


static int ssonic1_light=0;
static int ssonic2_light=0;

unsigned char *video_frame = NULL;

/*
Forward
Stop

*/


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

    close(fd);
}

static void start_all()
{
    mc_thread = g_thread_new("Guard", multicast_guard, NULL);
    sw_thread = g_thread_new("Watching", start_watching, NULL);
}

static void safe_exit()
{
    close_mc_fd();
    close_sw_fd();
    gtk_main_quit ();
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

static gboolean update_video(gpointer data)
{
    if(!camera_win_created)  return FALSE;

    if(camera_win==NULL)    return FALSE;

    if(is_video_update_timer_active()==0)  return FALSE;

    GdkPixbuf *pixbuf_img;
    static guint null_video_tms=0;

    if(null_video_tms==0)
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(noimg_1_xpm);
    else if(null_video_tms==1)    
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(noimg_2_xpm);
    else if(null_video_tms==2)    
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(noimg_3_xpm);
    else
        pixbuf_img = gdk_pixbuf_new_from_xpm_data(noimg_4_xpm);

    gtk_image_set_from_pixbuf(GTK_IMAGE(data), pixbuf_img);
    g_object_unref(G_OBJECT(pixbuf_img));

    null_video_tms = (null_video_tms+1)&0x3;

    return TRUE;    
}

static void base_set_win_style(GtkWidget *s_win)
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

    if(g_status==0xf)
        return TRUE;

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


static gboolean update_map(gpointer data)
{
    GdkPixbuf *pixbuf_img;

    pixbuf_img = gdk_pixbuf_new_from_file("./map.png", NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image_map), pixbuf_img);

    return TRUE;
}


static void create_camera_win(GtkWidget* pWindow)
{
    if(camera_win_created)  return;

    GdkPixbuf *pixbuf_img;
    GtkWidget *img;

    camera_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(noimg_1_xpm);
    img = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_container_add (GTK_CONTAINER (camera_win), img);
    gtk_window_set_title (GTK_WINDOW (camera_win), "Pop Up window");
    gtk_container_set_border_width (GTK_CONTAINER (camera_win), 0);
    gtk_window_set_resizable (GTK_WINDOW (camera_win), FALSE);
    gtk_window_set_decorated (GTK_WINDOW (camera_win), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (camera_win), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (camera_win), TRUE);
    gtk_widget_set_size_request (camera_win, 640, 480);
    gtk_window_set_transient_for (GTK_WINDOW (camera_win), GTK_WINDOW (pWindow));
    gtk_window_move (GTK_WINDOW (camera_win), gdk_screen_width(), 0);
    g_timeout_add(100, (GSourceFunc)update_video, (gpointer)img);
    start_video_update_timer();

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
        send_camera_command(DEVICE_ON);
        create_camera_win(pWindow);
        camera_win_created = TRUE;
    }
    else
    {
        send_camera_command(DEVICE_ON);
        gtk_widget_destroy(camera_win);
        camera_win_created = FALSE;
    }
}

static void system_setting(GtkButton* button, GtkWidget* pWindow)
{
/*    const char *url = "http://api.map.baidu.com/staticimage?width=960&height=600&center=118.802891,32.064735&zoom=11&ak=e7NXn4GEhwntTcwSowGwC6Ev";
    xmlNanoHTTPInit	();

    xmlNanoHTTPFetch (url, "./map.png", NULL);
    xmlNanoHTTPCleanup ();
*/
    GtkWidget *setting = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(setting), TRUE);
    gtk_window_set_default_size (GTK_WINDOW(setting), 400,300);
    gtk_window_set_title(GTK_WINDOW(setting), "Setting");
    gtk_window_set_has_resize_grip(GTK_WINDOW(setting), FALSE);
    gtk_widget_show_all(setting);
    
}

static void confirm_quit_dlg(GtkButton* button, GtkWidget* pWindow)
{
    gint choice;
    GtkWidget *content_area;

    GtkWidget *dialog = gtk_dialog_new_with_buttons ("Confirm",
                                                  pWindow,
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  "_Yes",
                                                  GTK_RESPONSE_YES,
                                                  "_No",
                                                  GTK_RESPONSE_NO,
                                                  NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
    gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new (""));
    gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new ("Really  Quit?"));
    gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new (""));
    gtk_widget_show_all(dialog);

    choice = gtk_dialog_run (GTK_DIALOG (dialog));
    if(choice==GTK_RESPONSE_YES)
        safe_exit();
    else
        gtk_widget_destroy(GTK_WIDGET(dialog));
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

/*    g_signal_connect (G_OBJECT (button_quadmotor),
                    "clicked",
                    G_CALLBACK (start_all),
                    (gpointer) m_win);
*/
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
    pixbuf_img = gdk_pixbuf_new_from_xpm_data(img19_xpm);
    img_camera = gtk_image_new_from_pixbuf(pixbuf_img);
    gtk_button_set_image(GTK_BUTTON(button_camera), img_camera);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_setting = gtk_button_new();
    g_signal_connect (G_OBJECT (button_setting),
                    "clicked",
                    G_CALLBACK (system_setting),
                    (gpointer) m_win);
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
    
    g_signal_connect (button_exit, "clicked", G_CALLBACK (confirm_quit_dlg), (gpointer) m_win);
    gtk_container_set_border_width(GTK_CONTAINER(button_exit), 0);

    gtk_container_set_border_width (GTK_CONTAINER(button_box), 0);

    return button_box;
}

GtkWidget *create_central_box()
{
    GtkWidget *mid_box;
    GtkWidget *top_status;
    GtkWidget *pane;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;
    GtkWidget *odometer;
    GtkWidget *frame;
    GtkWidget *align;
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
    image_map = gtk_image_new_from_pixbuf (pixbuf_img);
    gtk_container_add (GTK_CONTAINER (frame), image_map);
    g_object_unref(G_OBJECT(pixbuf_img));

    button_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);

    button1 = gtk_button_new_with_label ("Hi1 there");
    button2 = gtk_button_new_with_label ("Hi2 there");
    button3 = gtk_button_new_with_label ("Hi3 there");
//    odometer = odometer_new ();
    gtk_box_pack_start (GTK_BOX (button_box), button1, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (button_box), button2, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (button_box), button3, TRUE, TRUE, 0);
//    gtk_box_pack_start (GTK_BOX (button_box), odometer, TRUE, TRUE, 0);
    gtk_paned_pack2(GTK_PANED(pane), button_box, TRUE, TRUE);

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
    GtkWidget *overlay;
    GtkWidget *align;

	if (!g_thread_supported ()){ g_thread_init(NULL); }

	gdk_threads_init();
	gdk_threads_enter();

    gtk_init (&argc, &argv);

#if 0
m_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    m_box = odometer_new ();
    gtk_container_add (GTK_CONTAINER (m_win), m_box);
gtk_window_set_default_size(GTK_WINDOW(m_win),400, 400);
    g_signal_connect (m_win, "destroy", G_CALLBACK (gtk_main_quit), &m_win);

#else
    GtkWidget *odometer;

    m_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gdk_rgba_parse(&rgba_color, "#aaaaaa");
    gtk_widget_override_background_color(m_win, GTK_STATE_NORMAL, &rgba_color);

    g_signal_connect (m_win, "destroy", G_CALLBACK (safe_exit), &m_win);

    base_set_win_style(m_win);

    overlay = gtk_overlay_new ();
    m_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (overlay), m_box);
    gtk_container_add (GTK_CONTAINER (m_win), overlay);
    align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (align),
		     0, 7, 0, 7);
    odometer = odometer_new ();
    gtk_container_add (GTK_CONTAINER (align), odometer);

    gtk_overlay_add_overlay (GTK_OVERLAY (overlay), align);
    gtk_widget_set_halign (align, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (align, GTK_ALIGN_START);
    gtk_widget_show_all (overlay);

    left_box = create_left_box();
    gtk_box_pack_start (GTK_BOX (m_box), left_box, FALSE, FALSE, 5);

    mid_box = create_central_box();
    gtk_box_pack_start (GTK_BOX (m_box), mid_box, TRUE, FALSE, 0);

    right_box = create_right_box();
    gtk_box_pack_start (GTK_BOX (m_box), right_box, FALSE, FALSE, 0);

#endif

    get_my_own_addr();

    if (!gtk_widget_get_visible (m_win))
    {
        gtk_widget_show_all (m_win);
        gtk_window_maximize(GTK_WINDOW(m_win));
    }
    else
    {
        gtk_widget_destroy (m_win);
        m_win = NULL;
    }

    start_all();
    gtk_main ();
    
    gdk_threads_leave();

    return 0;
}

