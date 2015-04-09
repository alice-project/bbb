#include <cairo.h>
#include <gtk/gtk.h>

void do_drawing(cairo_t *, GtkWidget *);

struct {
  gboolean timer; 
  gdouble alpha;
  gdouble size;
} glob;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{      
  do_drawing(cr, widget);

  return FALSE;
}

void do_drawing(cairo_t *cr, GtkWidget *widget)
{
  cairo_text_extents_t extents;

  GtkWidget *win = gtk_widget_get_toplevel(widget);
  
  gint width, height;
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);  
  
  gint x = width/2;
  gint y = height/2;
  
  cairo_set_source_rgb(cr, 0.5, 0, 0); 
  cairo_paint(cr);   

  cairo_select_font_face(cr, "Courier",
      CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);
 
  glob.size += 0.8;

  if (glob.size > 20) {
      glob.alpha -= 0.01;
  }

  cairo_set_font_size(cr, glob.size);
  cairo_set_source_rgb(cr, 1, 1, 1); 

  cairo_text_extents(cr, "ZetCode", &extents);
  cairo_move_to(cr, x - extents.width/2, y);
  cairo_text_path(cr, "ZetCode");
  cairo_clip(cr);

  cairo_paint_with_alpha(cr, glob.alpha);
  
  if (glob.alpha <= 0) {
      glob.timer = FALSE;
  }     
}

static gboolean time_handler(GtkWidget *widget)
{ 
  if (!glob.timer) return FALSE;
  gtk_widget_queue_draw(widget);

  return TRUE;
}


int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;  
  
  glob.timer = TRUE;
  glob.alpha = 1.0;
  glob.size = 1.0;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);
 
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 350, 200); 
  gtk_window_set_title(GTK_WINDOW(window), "Puff");

  g_timeout_add(14, (GSourceFunc) time_handler, (gpointer) window);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
