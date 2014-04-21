#!/bin/bash

echo MA-GPIO > /sys/devices/bone_capemgr.9/slots

echo 5 > /sys/class/gpio/export
echo 65 > /sys/class/gpio/export
echo 105 > /sys/class/gpio/export

/root/robot

