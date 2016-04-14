#!/bin/sh

export DOMAIN="192.168.1.140"
export ADDR=http://$DOMAIN/

check_ip_timeout_vendor(){
    #$1 timeout(s)
    let timeout=0
    while [ $timeout -lt $1 ]
    do
        ping -c 2 $DOMAIN
        [ $? -eq 0 ] && return 0
        let timeout=$timeout+1
        sleep 1
    done
    return 1
}

check_network_vendor(){
    check_ip_timeout_vendor 5
    if [ $? -ne 0 ];then
        #restart the wifi
        /etc/wifi/wifi restart
        sleep 2
        /etc/wifi/udhcpc_wlan0 restart
        sleep 2
        check_ip_timeout_vendor 10
        [ $? -ne 0 ] && {
            echo "the network is not available"
            return 1
        }
        return 0
    fi
}
download_image_vendor(){
    # $1 image name  $2 DIR  $@ others
    rm -rf $2/$1
    echo "wget $ADDR/$1"
    wget $ADDR/$1 -P $2
}
upgrade_start_vendor(){
    # $1 mode: upgrade_pre,boot-recovery,upgrade_post
    #return   0 -> start upgrade;  1 -> no upgrade
    #reutrn value only work in nornal mode
    #nornal mode: $NORMAL_MODE
    echo upgrade_start_vendor $1
    return 0
}
upgrade_finish_vendor(){
    #set version
    write_misc -v henrisk_test_v1 -s ok
    #reboot -f
}
