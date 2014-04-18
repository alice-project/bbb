#include <gtk/gtk.h>
#include <glade/glade.h>


on_back_clicked (GtkToolButton*);
on_forward_clicked (GtkToolButton*);
on_up_clicked (GtkToolButton*);
on_refresh_clicked (GtkToolButton*);
on_delete_clicked (GtkToolButton*);
on_home_clicked (GtkToolButton*);
on_info_clicked (GtkToolButton*);
on_go_clicked (GtkButton*);
on_location_activate (GtkEntry*);
on_row_activated (GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*);


int main (int argc,
char *argv[])
{
    GtkWidget *window;
    GladeXML *xml;
    gtk_init (&argc, &argv);
    xml = glade_xml_new ("mui.glade", NULL, NULL);
    window = glade_xml_get_widget (xml, "top_window");
    glade_xml_signal_autoconnect (xml);
    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
