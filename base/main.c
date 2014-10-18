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
#include <gdk/gdkkeysyms.h>


int main(int argc, char* argv[])
{
//    if (!g_thread_supported())
//        g_thread_init(NULL);
    
    gdk_threads_init();

    gdk_threads_enter();  
    gtk_init(&argc, &argv);
    
    gtk_main ();
    gdk_threads_leave ();
    return 0;
}
