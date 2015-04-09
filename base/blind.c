#include <cairo.h>
#include <gtk/gtk.h>


static void do_drawing(cairo_t *);

struct {
  cairo_surface_t *image;
  cairo_surface_t *surface;
  gboolean timer;
  gint img_width;
  gint img_height;
} glob;


static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{      
  do_drawing(cr);
  
  return FALSE;
}

static gboolean time_handler(GtkWidget *widget)
{
  if (!glob.timer) return FALSE;
  
  gtk_widget_queue_draw(widget);
  return TRUE;
}

static void do_drawing(cairo_t *cr)
{
  cairo_t *ic;    
  static gint h = 0;     
  
  ic = cairo_create(glob.surface);

  cairo_rectangle(ic, 0, 0, glob.img_width, h);
  cairo_fill(ic);

  h += 1;
  if ( h == glob.img_height) glob.timer = FALSE;
  
  cairo_set_source_surface(cr, glob.image, 10, 10);
  cairo_mask_surface(cr, glob.surface, 10, 10);
  
  cairo_destroy(ic);  
}

static void init_vars()
{  
  glob.timer = TRUE;
  glob.image = cairo_image_surface_create_from_png("mask.png");  
  glob.img_width = cairo_image_surface_get_width(glob.image);
  glob.img_height = cairo_image_surface_get_height(glob.image);  
  glob.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                     glob.img_width, glob.img_height);
}

static void cleanup()
{
  cairo_surface_destroy(glob.image);
  cairo_surface_destroy(glob.surface);   
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;    

  gtk_init(&argc, &argv);
  
  init_vars();

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 
  g_signal_connect(G_OBJECT(window), "destroy",
      G_CALLBACK(gtk_main_quit), NULL);
 
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 325, 250); 
  gtk_window_set_title(GTK_WINDOW(window), "Blind down");

  g_timeout_add(15, (GSourceFunc) time_handler, (gpointer) window);

  gtk_widget_show_all(window);

  gtk_main();
  
  cleanup();

  return 0;
}
