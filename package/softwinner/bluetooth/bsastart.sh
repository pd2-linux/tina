#!/bin/sh
# $1: config file path
echo 0 > /sys/class/rfkill/rfkill0/state
sleep 1
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 1

/etc/bluetooth/bsa_server -all=0 -d /dev/ttyS1 -p /lib/firmware/ap6212/bcm43438a0.hcd -r 12 &
sleep 2
/etc/bluetooth/app_bluetooth $1
