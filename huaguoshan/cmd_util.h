#ifndef __MCAST_H__
#define __MCAST_H__

int send_gc_command(int mode);
int send_light_command(guint32 cmds);
int send_motion_command(guint32 cmds);
int send_test_command();

#endif


