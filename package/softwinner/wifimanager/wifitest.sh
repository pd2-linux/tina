#!/bin/ash

wifi_connect_ap_test $1 $2
sleep 1

wifi_on_off_test
sleep 2
