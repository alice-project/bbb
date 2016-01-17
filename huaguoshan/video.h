#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <gtk/gtk.h>

guint is_video_update_timer_active();
void start_video_update_timer();
void stop_video_update_timer();

#endif

