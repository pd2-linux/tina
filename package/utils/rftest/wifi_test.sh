#!/bin/sh
insmod /lib/modules/3.4.39/bcmdhd.ko iface_name=wlan0 firmware_path=/etc/rftest/fw_bcm43438a0_mfg.bin nvram_path=/lib/firmware/nvram_ap6212.txt
sleep 2
ifconfig wlan0 up
sleep 1
./wl ver
