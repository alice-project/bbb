#ifndef __R_MOTOR_H__
#define __R_MOTOR_H__

int start_motor(int m);
int stop_motor(int m);

void motor_regist();
void motor_init();

int parser_motion_cmd(struct s_base_motion *cmd);
void *detect_left_speed(void *data);
void *detect_right_speed(void *data);

#endif

