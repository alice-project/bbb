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

static guint camera_update_timer = 0;

guint is_video_update_timer_active()
{
    return camera_update_timer;
}

void start_video_update_timer()
{
    camera_update_timer = 1;
}

void stop_video_update_timer()
{
    camera_update_timer = 0;
}


