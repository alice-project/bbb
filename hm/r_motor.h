#ifndef __R_MOTOR_H__
#define __R_MOTOR_H__

#define WHEEL_RADIUS  (32)

#define LEFT_SIDE  0
#define RIGHT_SIDE 1

int start_motor(int m);
int stop_motor(int m);

void motor_regist();
void motor_init();

int parser_motion_cmd(void *cmd);
void *detect_speed(void *data);

#endif

