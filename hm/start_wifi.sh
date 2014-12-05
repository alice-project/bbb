#! /bin/sh
### BEGIN INIT INFO
# Provides:          start wlan
# Required-Start:
### END INIT INFO

PATH=/sbin:/usr/sbin:/bin:/usr/bin

ip link set wlan0 up
#iwconfig wlan0 mode managed
#iwconfig wlan0 essid "Jack Bauer" key ABC1234567
iw wlan0 connect "Jack Bauer" key 0:ABC1234567
iw wlan0 set power_save off 
dhclient wlan0
:

