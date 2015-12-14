#!/bin/sh
echo 0 > /sys/class/rfkill/rfkill0/state
sleep 1
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 1

chmod 000 /sys/class/rfkill/rfkill0/state

cd /etc/bluetooth
./bsa_server -all=0 -d /dev/ttyS1 -p /lib/firmware/ap6212/bcm43438a0.hcd -r 12 &
sleep 2
./app_bluetooth
