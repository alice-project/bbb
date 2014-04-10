#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <time.h>

#include "map.h"

G_DEFINE_TYPE(AntsMap, ants_map, GTK_TYPE_DRAWING_AREA);

static void ants_map_redraw_canvas (AntsMap *);

static void ants_map_example_function (AntsMap *ants)
{
    AntsMapPrivate *priv;
    
    priv = ANTS_MAP_GET_PRIVATE(ants);
}

static gboolean ants_map_update (gpointer data)
{
    AntsMap *ants;
    AntsMapPrivate *priv;
    time_t timet;
    
    ants = ANTS_MAP(data);
    priv = ANTS_MAP_GET_PRIVATE(ants);
    /* update the time */
    time(&timet);
    priv->time = localtime(&timet);
    
    ants_map_redraw_canvas(ants);
    
    return TRUE; /* keep running this event */
}

static void ants_map_redraw_canvas(AntsMap *ants)
{
    GtkWidget *widget;
    GdkRegion *region;
    
    widget = GTK_WIDGET(ants);
    
    if (!widget->window) return;
    
    region = gdk_drawable_get_clip_region (widget->window);
    /* redraw the cairo canvas completely by exposing it */
    gdk_window_invalidate_region (widget->window, region, TRUE);
    gdk_window_process_updates (widget->window, TRUE);
    
    gdk_region_destroy (region);
}

double coord_dash[] = {10.0,10.0,10.0,10.0};
static void draw_bg_map(GtkWidget *map, cairo_t *cr)
{
    double x, y;
    guint i;

    /* main coordinate line */
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 255, 0, 0);
    x = map->allocation.width / 2;
    y = 0;
    cairo_move_to(cr, x, y);

    x = 0;
    y = map->allocation.height;
    cairo_rel_line_to(cr, x, y);

    cairo_stroke(cr);
    x = 0;
    y = map->allocation.height / 2;
    cairo_move_to(cr, x, y);

    x = map->allocation.width;
    y = 0;
    cairo_rel_line_to(cr, x, y);
    
    /* assit coordinate line */
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 255, 0);
    cairo_set_line_width(cr, 1);
    cairo_set_dash(cr, coord_dash, sizeof(coord_dash)/sizeof(coord_dash[0]), -20.0);
    for(i = 0;i < 10;i++)
    {
        if(i == 5) continue;
        
        x = 0 + i * map->allocation.width / 10;
        y = 0;
        cairo_move_to(cr, x, y);
    
        x = 0;
        y = map->allocation.height;
        cairo_rel_line_to(cr, x, y);
        
        cairo_stroke(cr);
    }
    for(i = 0;i < 10;i++)
    {
        if(i == 5) continue;
        
        x = 0;
        y = 0 + i * map->allocation.height / 10;
        cairo_move_to(cr, x, y);
    
        x = map->allocation.width;
        y = 0;
        cairo_rel_line_to(cr, x, y);
        
        cairo_stroke(cr);
    }

    cairo_stroke(cr);
}

#define  CHASSIS_LENGTH          30
#define  CHASSIS_WIDTH           20
#define  FRONT_WHEEL_RADIUS      5
#define  FRONT_WHEEL_WIDTH       2
#define  REAR_WHEEL_RADIUS       8
#define  REAR_WHEEL_WIDTH        2
double wheel_dash[] = {1.0,1.0,1.0,1.0};
void draw_an_ant(cairo_t *cr, double xc, double yc, double angle)
{
    double x, y;
    cairo_stroke(cr);
    cairo_set_line_width(cr, 0.5);
    cairo_set_source_rgb(cr, 90, 30, 90);
    x = xc - CHASSIS_WIDTH/2;
    y = yc - CHASSIS_LENGTH/2;
    cairo_rectangle(cr, x, y, CHASSIS_WIDTH, CHASSIS_LENGTH);
    cairo_fill (cr);

    /* left-front wheel */
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 255, 0);
    cairo_set_line_width(cr, 1);
    cairo_set_dash(cr, wheel_dash, sizeof(wheel_dash)/sizeof(wheel_dash[0]), -1.0);
    x = xc - CHASSIS_WIDTH*2/5;
    y = yc - CHASSIS_LENGTH*2/5;
    cairo_rectangle(cr, x, y, REAR_WHEEL_WIDTH, REAR_WHEEL_RADIUS*2);


    /* right-front wheel */
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 255, 0);
    cairo_set_line_width(cr, 1);
    cairo_set_dash(cr, wheel_dash, sizeof(wheel_dash)/sizeof(wheel_dash[0]), -1.0);
    x = xc + CHASSIS_WIDTH*2/5;
    y = yc - CHASSIS_LENGTH*2/5;
    cairo_rectangle(cr, x, y, REAR_WHEEL_WIDTH, REAR_WHEEL_RADIUS*2);

    /* left-rear wheel */
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 255, 0);
    cairo_set_line_width(cr, 1);
    cairo_set_dash(cr, wheel_dash, sizeof(wheel_dash)/sizeof(wheel_dash[0]), -1.0);
    x = xc - CHASSIS_WIDTH*2/5;
    y = yc + CHASSIS_LENGTH*2/5;
    cairo_rectangle(cr, x, y, REAR_WHEEL_WIDTH, REAR_WHEEL_RADIUS*2);

    /* right-front wheel */
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 255, 0);
    cairo_set_line_width(cr, 1);
    cairo_set_dash(cr, wheel_dash, sizeof(wheel_dash)/sizeof(wheel_dash[0]), -1.0);
    x = xc + CHASSIS_WIDTH*2/5;
    y = yc + CHASSIS_LENGTH*2/5;
    cairo_rectangle(cr, x, y, REAR_WHEEL_WIDTH, REAR_WHEEL_RADIUS*2);
}

void draw(GtkWidget *map, cairo_t *cr)
{
}

static gboolean ants_map_expose (GtkWidget *map, GdkEventExpose *event)
{
    cairo_t *cr;
    double xc, yc;

    xc = map->allocation.width / 2;
    yc = map->allocation.height / 2;
    /* get a cairo_t */
    cr = gdk_cairo_create(map->window);

    /* set a clip region for the expose event */
    cairo_rectangle(cr, event->area.x, event->area.y,
                        event->area.width, event->area.height);
    cairo_clip(cr);

    draw_bg_map(map, cr);
    draw_an_ant(cr, xc, yc, 0.0);

    cairo_destroy(cr);

    return FALSE;
}


static void ants_map_class_init(AntsMapClass *map_class)
{
    GtkWidgetClass *widget_class;
    
    widget_class = GTK_WIDGET_CLASS (map_class);
    
    widget_class->expose_event = ants_map_expose;
    g_type_class_add_private(map_class, sizeof (AntsMapPrivate));
}

static void ants_map_init(AntsMap *map)
{
     ants_map_update(map);

     /* update the clock once a second */
     g_timeout_add (1000, ants_map_update, map);
}

GtkWidget* ants_map_new(void)
{
    return g_object_new(ANTS_MAP_TYPE, NULL);
}
