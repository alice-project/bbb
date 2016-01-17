#ifndef __MCAST_H__
#define __MCAST_H__

int send_gc_command(int mode);
int send_light_command(unsigned int cmds);
int send_motion_command(struct s_base_motion *motion);
int send_test_command();

#endif


