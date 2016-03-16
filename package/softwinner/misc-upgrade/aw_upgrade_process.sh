#!/bin/sh

. /sbin/aw_upgrade_vendor.sh

echo ..... haha2 $1

#DOMAIN="we.china-ota.com"
#ADDR=http://$DOMAIN/WeChatSurvey/

#test

RAMDISK_IMG=ramdisk_sys.tar.gz
TARGET_IMG=target_sys.tar.gz
USR_IMG=usr_sys.tar.gz

UPGRADE_SH=/sbin/aw_upgrade_image.sh

check_ip_timeout_default(){
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
check_network_default(){
    check_ip_timeout_default 5
    if [ $? -ne 0 ];then
        #restart the wifi
        /etc/init.d/wifi start
        sleep 2
        /etc/init.d/udhcpc_wlan0 start
        sleep 2
        check_ip_timeout_default 10
        [ $? -ne 0 ] && {
            echo "the network is not available"
            exit 1;
        }
    fi
}

download_image(){
    # $1 image name  $2 DIR
    type download_image_vendor 1>/dev/null 2>/dev/null
    [ $? -ne 0 ] && {
        echo "vendor download image is available!"
        exit 1
    }
    download_image_vendor $1 $2
}
try_mount(){
    # $1 partition name $2 mount dir
    format_list="ext4 jffs2 vfat"
    for i in $format_list; do
        echo "mounting $i /dev/by-name/$1 -> $2"
        mount -t $i /dev/by-name/$1 $2
        [ $? -eq 0 ] && break
    done
}
check_network(){
    type check_network_vendor 1>/dev/null 2>/dev/null 
    [ $? -ne 0 ] && {
        echo "vendor check network is no available!"
        exit 1
    }

    check_network_vendor 
    [ $? -ne 0 ] && {
        exit 1
    }
}
set_version(){
    # $1 version
    $UPGRADE_SH version $1
}
upgrade_finish(){
    type upgrade_finish_vendor 1>/dev/null 2>/dev/null 
    [ $? -ne 0 ] || {
        upgrade_finish_vendor
    }
}
download_prepare_image(){
    # $1 image
    download_image $1 /tmp
    #md5 check for the package
    #if check ok
    $UPGRADE_SH prepare /tmp/$1
}
boot_recovery_mode(){
    # boot-reocvery mode
    # -->get target_sys.tar.gz
    # ---->write image to "boot", "rootfs", "extend" partition
    echo "boot_recovery_mode"

    # umount the usr partition; if failed, ignore
    umount /usr
   
    #try to mount rootfs_data partition
    try_mount "rootfs_data" "/overlay"
    try_mount "UDISK" "/mnt/UDISK"

    # get current wifi wpa_supplicant.conf
    [ -f /overlay/etc/wifi/wpa_supplicant.conf ] && {
        echo "get wpa_supplicant from overlay"
        echo "old wpa_supplicant.config"
        cat /etc/wifi/wpa_supplicant.conf
        cp /overlay/etc/wifi/wpa_supplicant.conf /etc/wifi/
        echo "new wpa_supplicant.config"
        cat /etc/wifi/wpa_supplicant.conf
    }

    check_network

    $UPGRADE_SH clean
    
    download_prepare_image $TARGET_IMG

    $UPGRADE_SH upgrade
}
upgrade_pre_mode(){
    # upgrade_pre mode
    # -->get ramdisk_sys.tar.gz target_sys.tar.gz
    # ---->write ramdisk to "extend"
    # ------>write image to "boot", "rootfs", "extend" partition
    echo "upgrade_pre_mode"

    # umount the usr partition; if failed, ignore
    umount /usr
    
    check_network

    $UPGRADE_SH clean
    
    download_prepare_image $RAMDISK_IMG
    download_prepare_image $TARGET_IMG
    
    $UPGRADE_SH upgrade
}
upgrade_post_mode(){
    # upgrade_post mode
    # -->get usr_sys.tar.gz
    # ---->write image to "extend" partition 
    echo "upgrade_post_mode"

    check_network

    $UPGRADE_SH clean
    
    download_prepare_image $USR_IMG
    
    $UPGRADE_SH upgrade

}
upgrade_end_mode(){
    # upgrade_end mode
    # wait for next upgrade
    echo "wait for next upgrade!!"
}
####################################################
# force to upgrade
if [ -n $1 ] && [ x$1 = x"--force" ]; then
    upgrade_pre_mode
    upgrade_finish
    exit 0
fi
system_flag=`read_misc command`
if [ x$system_flag = x"boot-recovery" ]; then
    boot_recovery_mode
    upgrade_finish
elif [ x$system_flag = x"upgrade_pre" ]; then
    upgrade_pre_mode
    upgrade_finish
elif [ x$system_flag = x"upgrade_post" ]; then
    upgrade_post_mode
    upgrade_finish
elif [ x$system_flag = x"upgrade_end" ]; then
    upgrade_end_mode
else
    upgrade_end_mode
fi
