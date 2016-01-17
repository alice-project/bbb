#ifndef __FOOT_H__
#define __FOOT_H__

#include "pub.h"

class XZ_Foot {
public:
    XZ_Foot();
    XZ_Foot(int side);
    ~XZ_Foot();

private:
    void measure();
    void adjust();

    u_int32 speed_mm;
    u_int32 speed_pulse;
    u_int32 pwm;
    int direction;
    int m_side;

    int dir_pin_0;
    int dir_connector_0;
    int dir_pin_1;
    int dir_connector_1;
    int pwm_pin;
    int pwm_connector;

protected:

public:
    int init();
    u_int32 get_speed_pulse(){return speed_pulse;}
    u_int32 get_speed_mm(){return speed_mm;}
    void set_speed_to(u_int32 speed);  // speed_mm
    void set_direction(int d);
};


#endif

