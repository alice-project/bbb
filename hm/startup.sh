#!/bin/bash

echo 5 > /sys/class/gpio/export
echo 65 > /sys/class/gpio/export
echo 105 > /sys/class/gpio/export

echo BONE-ROBOT-HM > /sys/devices/bone_capemgr.9/slots

/home/ubuntu/robot/robot&


