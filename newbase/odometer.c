#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "odometer.h"

/* Properties enum */
enum {
    PROP_0, /* Padding */
    PROP_SPEED,
    PROP_MILEAGE,
    PROP_TIME,
};


/* Private data structure */
struct _OdometerPrivate {
    GdkWindow *main_win;

    gdouble speed;
    gdouble mileage;
    guint   hour;
    guint   min;
};

const gint WIDTH = 200;
const gint HEIGHT = 200;


/* Internal API */
static void odometer_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void odometer_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void odometer_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void odometer_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void odometer_realize(GtkWidget *widget);
static void odometer_set_time(Odometer *odometer, gdouble value);
static gboolean odometer_draw (GtkWidget *widget, cairo_t *cr);

G_DEFINE_TYPE (Odometer, odometer, GTK_TYPE_WIDGET)

/* Initialization */
static void odometer_class_init(OdometerClass *klass) {
    GObjectClass   *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GObjectClass *)klass;
    widget_class = (GtkWidgetClass *)klass;

    object_class->set_property = odometer_set_property;
    object_class->get_property = odometer_get_property;
    widget_class->draw = odometer_draw;
//    widget_class->realize = odometer_realize;

    g_object_class_install_property (object_class,
                                   PROP_SPEED,
                                   g_param_spec_int ("speed",
                                                     "Speed",
                                                     "Current Speed",
                                                     0, 100, 0,
                                                     G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                   PROP_MILEAGE,
                                   g_param_spec_int ("milemeter",
                                                     "milemeter",
                                                     "Mile Meter",
                                                     0, 99999, 0,
                                                     G_PARAM_READWRITE));
}

static void odometer_init(Odometer *odometer) {
    GtkWidget *widget = GTK_WIDGET (odometer);
    OdometerPrivate *priv;

    priv = odometer->priv = odometer_get_instance_private (odometer);
    gtk_widget_set_can_focus (widget, TRUE);
    gtk_widget_set_has_window (widget, FALSE);
}

/* Overriden virtual methods */
static void odometer_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec) 
{
    Odometer *odometer = ODOMETER(object);
    switch(prop_id) 
    {
        case PROP_SPEED:
            odometer_set_speed(odometer, g_value_get_double(value));
            break;
        case PROP_MILEAGE:
            odometer_set_mileage(odometer, g_value_get_double(value));
            break;
        case PROP_TIME:
            odometer_set_time(odometer, g_value_get_double(value));
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void odometer_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec) 
{
    Odometer *odometer = ODOMETER(object);
    OdometerPrivate *priv = odometer->priv;
    switch(prop_id) 
    {
        case PROP_SPEED:
            g_value_set_double(value, priv->speed);
            break;
        case PROP_MILEAGE:
            g_value_set_double(value, priv->mileage);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void odometer_realize(GtkWidget *widget) 
{
    OdometerPrivate *priv = ODOMETER(widget)->priv;
    GtkAllocation alloc;
    GdkWindowAttr attrs;
    guint attrs_mask;

    gtk_widget_set_realized(widget, TRUE);
    gtk_widget_get_allocation(widget, &alloc);

    attrs.x = alloc.x;
    attrs.y = alloc.y;
    attrs.width = alloc.width;
    attrs.height = alloc.height;
    attrs.window_type = GDK_WINDOW_CHILD;
    attrs.wclass = GDK_INPUT_OUTPUT;
    attrs.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attrs_mask = GDK_WA_X | GDK_WA_Y;
    priv->main_win = gdk_window_new(gtk_widget_get_window (widget),
                                   &attrs, attrs_mask);
    gdk_window_set_user_data(priv->main_win, widget);
    gtk_widget_set_window(widget, priv->main_win);

    time_t rawtime;
    time(&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    priv->mileage = 0;
    priv->speed = 0;
    priv->hour = timeinfo->tm_hour;
    priv->min = timeinfo->tm_min;

}

static void odometer_size_request(GtkWidget *widget,
    GtkRequisition *requisition) {
    requisition->width = WIDTH;
    requisition->height = HEIGHT;
}

static void odometer_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation) {
    OdometerPrivate *priv;
    priv = ODOMETER(widget)->priv;
    gtk_widget_set_allocation(widget, allocation);
    if (gtk_widget_get_realized(widget)) {
        gdk_window_move_resize(priv->main_win, allocation->x, allocation->y, WIDTH, HEIGHT);
    }
}

static void odometer_draw_mileage(cairo_t *cr, int width, int height)
{
    gdouble line_width=0.001;
    gdouble radius = 0.15;

    cairo_set_line_width(cr, line_width);
    cairo_set_source_rgba(cr, 71.0/255.0, 70.0/255.0, 78.0/255.0, 1.0);
    cairo_arc(cr, 0, 0, radius, 0, 2 * M_PI);
//    cairo_set_source_rgba(cr, 147.0/255.0, 148.0/255.0, 143.0/255.0, 0.9);
    cairo_paint(cr);
}

static void digit_font(cairo_t *cr, guint num, int lt_x, int lt_y)
{
//    cairo_pattern_t *pat;
//    pat = cairo_pattern_create_rgba (173.0/255.0, 190.0/255.0, 155.0/255.0, 0.9);
//    cairo_set_source (cr, pat);
    cairo_set_line_width(cr, 0.1);
    cairo_save(cr);
    cairo_set_source_rgba(cr, 17.0/255.0, 27.0/255.0, 39.0/255.0, 0.9);

    switch (num)
    {
        case 0:
            cairo_move_to (cr, lt_x, lt_y);
            cairo_line_to (cr, lt_x-0.15*sin(M_PI*5/360), lt_y-0.15*tan(M_PI*5/360));
            
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            break;
    }
//    cairo_pattern_destroy (pat);
cairo_restore(cr);
}

static gboolean format_mileage(guint num, guchar str[])
{
    int i=0;

    num = num % 1000000;
    while(num > 0)
    {
        str[5-(i++)] = '0' + (num % 10);
        num = num / 10;
    }
    return 0;
}

static void display_mileage(GtkWidget *widget, cairo_t *cr, gdouble x, gdouble y)
{
    guchar str_mileage[8];
    OdometerPrivate *priv = ODOMETER(widget)->priv;
    cairo_pattern_t *pat;
    cairo_text_extents_t extents;

    memset(str_mileage, '0', sizeof(str_mileage));
    str_mileage[6] = 'm';
    str_mileage[7] = '\0';
    format_mileage(priv->mileage, str_mileage);
    pat = cairo_pattern_create_rgba (173.0/255.0, 190.0/255.0, 155.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_select_font_face (cr, "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 0.05);
    cairo_text_extents (cr, str_mileage, &extents);
    cairo_move_to (cr, x, y);
    cairo_show_text (cr, str_mileage);
    cairo_pattern_destroy (pat);
}

static void display_time(GtkWidget *widget, cairo_t *cr, gdouble x, gdouble y)
{
    guchar str_time[9];
    OdometerPrivate *priv = ODOMETER(widget)->priv;
    cairo_pattern_t *pat;
    cairo_text_extents_t extents;

    memset(str_time, 0, sizeof(str_time));
    time_t rawtime;
    time(&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    sprintf(str_time, "%2d:%2d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    pat = cairo_pattern_create_rgba (173.0/255.0, 190.0/255.0, 155.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_select_font_face (cr, "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 0.05);
    cairo_text_extents (cr, str_time, &extents);
    cairo_move_to (cr, x, y);
    cairo_show_text (cr, str_time);
    cairo_pattern_destroy (pat);
}

static void draw_tick(GtkWidget *widget, cairo_t *cr)
{
    gdouble x, y;
    gdouble len = 0.045;
    gdouble radius = 0.39;
    gdouble angle;
    int i = 0;
    cairo_text_extents_t extents;
    guchar str[4];
    cairo_pattern_t *pat;

    cairo_set_source_rgba(cr, 72.0/255.0, 109.0/255.0, 251.0/255.0, 0.9);
    cairo_set_line_width(cr, 0.01);

    for(i = 0;i < 31; i++) {
        angle = 30.0 + i * 10.0;
        if((i%3)==0) {
            x = -(radius+len/2) * sin(2*M_PI*angle/360.0);
            y = (radius+len/2) * cos(2*M_PI*angle/360.0);
            cairo_arc(cr, x, y, 0.015, 0, 2 * M_PI);
            cairo_fill (cr);

            sprintf(str, "%d", i*5);
            x = -(radius-len/2-i*(0.002)) * sin(2*M_PI*(angle-3.0)/360.0);
            y = (radius-len/2-i*(0.002)) * cos(2*M_PI*(angle-3.0)/360.0);
            pat = cairo_pattern_create_rgba (72.0/255.0, 109.0/255.0, 251.0/255.0, 0.9);
            cairo_set_source (cr, pat);
            cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size (cr, 0.05);
            cairo_text_extents (cr, str, &extents);
            cairo_move_to (cr, x, y);
            cairo_show_text (cr, str);
            cairo_pattern_destroy (pat);
        } else {
            x = -radius * sin(2*M_PI*angle/360.0);
            y = radius * cos(2*M_PI*angle/360.0);
            cairo_move_to(cr, x, y);        
            cairo_line_to(cr, x-(len*sin(M_PI*angle*2/360)), y+(len*cos(M_PI*angle*2/360)));
            cairo_stroke (cr);
        }
    }
}

static gboolean draw_pointer(GtkWidget *widget, cairo_t *cr)
{

}

static gboolean 
odometer_draw (GtkWidget *widget,
               cairo_t   *cr)
{
    OdometerPrivate *priv = ODOMETER(widget)->priv;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    const int width = allocation.width;
    const int height = allocation.height;
    cairo_pattern_t *pat;
    gdouble radius = 0.45;
    cairo_text_extents_t extents;

    cairo_scale(cr, width, height);
    cairo_translate(cr, 0.5, 0.5);

    /* the most outside circle */
    pat = cairo_pattern_create_rgba (45.0/255.0, 46.0/255.0, 64.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_arc (cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_pattern_destroy (pat);

    pat = cairo_pattern_create_rgba (30.0/255.0, 34.0/255.0, 46.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_arc (cr, 0, 0, 0.38, 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_pattern_destroy (pat);

    pat = cairo_pattern_create_rgba (155.0/255.0, 154.0/255.0, 150.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_arc (cr, 0, 0, 0.23, 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_pattern_destroy (pat);

    pat = cairo_pattern_create_rgba (73.0/255.0, 90.0/255.0, 255.0/255.0, 0.9);
    cairo_set_source (cr, pat);
    cairo_select_font_face (cr, "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 0.05);
    cairo_text_extents (cr, "cm/sec", &extents);
    cairo_move_to (cr, -0.08, 0.35);
    cairo_show_text (cr, "cm/sec");
    cairo_pattern_destroy (pat);
    display_time(widget, cr, -0.1, 0.02);
    display_mileage(widget, cr, -0.12, 0.10);
    draw_tick(widget, cr);
/*
    pat = cairo_pattern_create_radial (width/2, height/2, 25.6,
                                       width/2, height/2, 128.0);
    cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
    cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
    cairo_set_source (cr, pat);
    cairo_arc (cr, width/2, height/2, 76.8, 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_pattern_destroy (pat);
/*
cairo_set_source_rgba(cr, 45.0/255.0, 46.0/255.0, 64.0/255.0, 0.9);
cairo_set_line_width(cr, line_width*10);
cairo_arc(cr, 0, 0, radius*1.3, 0, 2 * M_PI);
*/
//    cairo_set_line_width(cr, line_width);
//    cairo_save(cr);
//    cairo_set_source_rgba(cr, 17.0/255.0, 27.0/255.0, 39.0/255.0, 0.9);
//    cairo_paint(cr);

//odometer_draw_mileage(cr, width, height);
/*
    cairo_restore(cr);
    cairo_arc(cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_fill_preserve(cr);
    cairo_restore(cr);
    cairo_stroke_preserve(cr);
    cairo_clip(cr);

    //clock ticks
    int i;
    for (i = 0; i < 12; i++)
    {
        double inset = 0.05;

        cairo_save(cr);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

        if(i % 3 != 0)
        {
            inset *= 0.8;
            cairo_set_line_width(cr, line_width);
        }

        cairo_move_to(cr, 
                (radius - inset) * cos (i * M_PI / 6),
                (radius - inset) * sin (i * M_PI / 6));
        cairo_line_to (cr, 
                radius * cos (i * M_PI / 6),
                radius * sin (i * M_PI / 6));
        cairo_stroke(cr);
        cairo_restore(cr);
    }

    // store the current time
    time_t rawtime;
    time(&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    // compute the angles of the indicators of our clock
    double minutes = timeinfo->tm_min * M_PI / 30;
    double hours = timeinfo->tm_hour * M_PI / 6;
    double seconds= timeinfo->tm_sec * M_PI / 30;

    cairo_save(cr);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // draw the seconds hand
    cairo_save(cr);
    cairo_set_line_width(cr, line_width);
    cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.8); // gray
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(seconds) * (radius * 0.9), -cos(seconds) * (radius * 0.9));
    cairo_stroke(cr);
    cairo_restore(cr);

    // draw the minutes hand
    cairo_set_source_rgba(cr, 0.117, 0.337, 0.612, 0.9);   // blue
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(minutes + seconds / 60) * (radius * 0.8),
                -cos(minutes + seconds / 60) * (radius * 0.8));
    cairo_stroke(cr);

    // draw the hours hand
    cairo_set_source_rgba(cr, 0.337, 0.612, 0.117, 0.9);   // green
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, sin(hours + minutes / 12.0) * (radius * 0.5),
                -cos(hours + minutes / 12.0) * (radius * 0.5));
    cairo_stroke(cr);
    cairo_restore(cr);
*/

    return TRUE;
}

/* Public API */
GtkWidget *odometer_new(void) {
    return(g_object_new(TYPE_ODOMETER, NULL));
}

gdouble odometer_get_speed(Odometer *odometer) {
    g_return_val_if_fail(IS_ODOMETER(odometer), 0);
    return(odometer->priv->speed);
}

void odometer_set_speed(Odometer *odometer, gdouble speed) {
    g_return_if_fail(IS_ODOMETER(odometer));
    odometer->priv->speed = speed;
    gtk_widget_queue_draw(GTK_WIDGET(odometer));
}


void odometer_set_time(Odometer *odometer, gdouble value) {
    g_return_if_fail(IS_ODOMETER(odometer));
    time_t rawtime;
    time(&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    odometer->priv->hour = timeinfo->tm_hour;
    odometer->priv->min = timeinfo->tm_min;
    gtk_widget_queue_draw(GTK_WIDGET(odometer));
    gtk_widget_queue_draw(GTK_WIDGET(odometer));
}

gdouble odometer_get_mileage(Odometer *odometer) {
    g_return_val_if_fail(IS_ODOMETER(odometer), 0);
    return(odometer->priv->mileage);
}

void odometer_set_mileage(Odometer *odometer, gdouble mileage) {
    g_return_if_fail(IS_ODOMETER(odometer));
    odometer->priv->mileage = mileage;
    gtk_widget_queue_draw(GTK_WIDGET(odometer));
}


