#ifndef __R_EVENT_H__
#define __R_EVENT_H__

#include "common.h"


enum R_MSG_ID {
/* MessageID 0~99 reserved */
R_MSG_INVALID = 0,
R_MSG_TEST,

/* MessageID 100~199 for DC Motor */
R_MSG_SET_DCMOTOR_SPEED = 100,


/* MessageID 200~299 for ServoMotor */


/* MessageID 300~399 for Sonic-sensor */


/* MessageID 400~499 for Timer */
R_MSG_USONIC_SCAN = 400,
R_MSG_SERVO_0,
R_MSG_MOTOR_SPEED,

};



#endif

