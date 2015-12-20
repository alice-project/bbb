#!/bin/bash

echo 5 > /sys/class/gpio/export
echo 65 > /sys/class/gpio/export
echo 66 > /sys/class/gpio/export
echo rising > /sys/class/gpio/gpio66/edge
echo 67 > /sys/class/gpio/export
echo 68 > /sys/class/gpio/export
echo 69 > /sys/class/gpio/export
echo 105 > /sys/class/gpio/export

echo BONE-ROBOT-HM > /sys/devices/bone_capemgr.9/slots

/home/ubuntu/robot/robot&


