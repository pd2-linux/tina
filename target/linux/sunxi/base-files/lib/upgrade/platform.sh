#!/bin/sh

TEMP=/tmp
CHECK_DIR=ota_md5
CHECK_FILE=ota.md5

OTA_PACKAGE=ota.tar.gz
OTA_DIR=ota
ROOTFS=rootfs.ext4

platform_check_image() {
	echo sunxi platform check image!!
	#check ota package md5
	[ ! -e $1 ] && return 1
	cd $TEMP && tar zxvf $1

	str1=`cat $TEMP/$CHECK_DIR/$CHECK_FILE | awk '{print $1}'`
	str2=`md5sum $TEMP/$CHECK_DIR/$OTA_PACKAGE | awk '{print $1}'`

	[ $str1 = $str2 ] && return 0
	echo md5 check faied! request: $str1,result:$str2
	return 1
}

platform_copy_config() {
	echo sunxi platform copy config!!
}

platform_do_upgrade() {
	echo sunxi platform do upgrade!!
	#now upgrade rootfs
	#todo: upgrade kernel boot ...
	[ -e /tmp/ota_md5 ] && cd /tmp/ota_md5 && { \
		tar -zxvf ota.tar.gz; \
		dd of=/dev/root if=/tmp/ota_md5/ota/rootfs.ext4 bs=1M; \
		return
	}
	echo sunxi upgrade failed!!

}
