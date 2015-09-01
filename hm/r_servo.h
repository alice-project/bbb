#ifndef __R_SERVO_H__
#define __R_SERVO_H__

int servo_0_rotating(void *data);
void servo_rotate_to(int id, double angle);
int regular_servo_rotating(void *data);
void *servo_proc(void *data);

void set_camera_rotate(int state);

#endif

