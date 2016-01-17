#include "foot.h"
#include "gpio.h"

XZ_Foot::XZ_Foot()
{
    init();
}

/* dir ctrl, dir ctrl, speed ctrl 
  {9, 11, 9, 12, 8, 13},        // Left DC MOTOR control & dir
  {9, 13, 9, 14, 8, 19},        // Right DC MOTOR control & dir
*/
XZ_Foot::XZ_Foot(int side)
{
    m_side = side;
    if(side == LEFT_SIDE) {
        pwm_connector = 8;
        pwm_pin = 13;
        dir_pin_0 = 9;
        dir_pin_1 = 9;
        dir_connector_0 = 11;
        dir_connector_1 = 11;
    } else {
        pwm_connector = 8;
        pwm_pin = 19;
        dir_pin_0 = 9;
        dir_pin_1 = 9;
        dir_connector_0 = 13;
        dir_connector_1 = 14;
    }

    init();    

}

XZ_Foot::~XZ_Foot()
{
}

int XZ_Foot::init()
{
    speed_mm = 0;
    speed_pulse = 0;

    regist_gpio(dir_connector_0, dir_pin_0, DIR_OUT);
    regist_gpio(dir_connector_1, dir_pin_1, DIR_OUT);

    regist_gpio(pwm_connector, pwm_pin, DIR_IN);
}

void XZ_Foot::set_direction(int d)
{
    if(d==POSITIVE_DIR) {
        set_pin_high(dir_pin_0, dir_connector_0);
        set_pin_low(dir_pin_1, dir_connector_1);
    } else {
        set_pin_low(dir_pin_0, dir_connector_0);
        set_pin_high(dir_pin_1, dir_connector_1);
    }
}

void XZ_Foot::set_speed_to(u_int32 speed)
{
}


