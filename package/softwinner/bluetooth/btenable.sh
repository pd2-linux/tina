#!/bin/ash
# $1: on or off

bt_on()
{
  echo 0 > /sys/class/rfkill/rfkill0/state
  sleep 1
  echo 1 > /sys/class/rfkill/rfkill0/state
  sleep 1

  echo bt pwd $1
  echo bt addr path $2
  cd $1

  /etc/bluetooth/bsa_server -all=0 -d /dev/ttyS1 -p /lib/firmware/ap6212/bcm43438a0.hcd -r 12 &
  sleep 2
  /etc/bluetooth/app_bluetooth $2 &
}

bt_off()
{
  killall app_bluetooth
  sleep 1
    
  killall bsa_server
  sleep 1
    
  echo 0 > /sys/class/rfkill/rfkill0/state
}

if [ "$1" = "on" ]; then
    echo "turn on bt"
    bt_on $2 $3
else
    if [ "$1" = "off" ]; then
        echo "turn off bt"
        bt_off
    else
        echo "no paras"
        exit 1
    fi
fi