# setenv script based on android envsetup.sh
# author:tracewong
# date: 2015-08-31

function print_red(){
	echo -e '\033[0;31;1m'
	echo $1
	echo -e '\033[0m'
}
function gettop
{
    local TOPFILE=scripts/mksetup.sh
    if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
        # The following circumlocution ensures we remove symlinks from TOP.
        (cd $TOP; PWD= /bin/pwd)
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            local HERE=$PWD
            T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
                \cd ..
                T=`PWD= /bin/pwd -P`
            done
            \cd $HERE
            if [ -f "$T/$TOPFILE" ]; then
                echo $T
            fi
        fi
    fi
}

function goroot()
{
    T=$(gettop)
    if [ "$T" ]; then
        \cd $(gettop)
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function gotarget()
{
    T=$(gettop)
    if [ "$T" ]; then
	if [ "x${CHIP}" = "xsun5i" ]; then
            \cd $(gettop)/target/linux/sun5i
    	else
            \cd $(gettop)/target/linux/sunxi
    	fi
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function gokernel()
{
    T=$(gettop)
    if [ "$T" ]; then
        \cd $(gettop)/lichee/$KERNEL_VER
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function goboot()
{
    T=$(gettop)
    if [ "$T" ]; then
	if [ "x${CHIP}" = "xsun5i" ]; then
            \cd $(gettop)/package/boot/uboot-sun5i
        else
            \cd $(gettop)/package/boot/uboot-sunxi
	fi
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function goconfig()
{
    T=$(gettop)
    if [ "$T" ]; then
	if [ "x${CHIP}" = "xsun5i" -o "x${CHIP}" = "xsun5iw1p2" ]; then
            \cd $(gettop)/package/boot/uboot-sun5i/bootloader/$CHIP/configs/$BOARD
    	else
            \cd $(gettop)/package/boot/uboot-sunxi/bootloader/$CHIP/configs/$BOARD
    	fi
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function godir()
{
    if [[ -z "$1" ]]; then
        echo "Usage: godir <regex>"
        return
    fi
    T=$(gettop)
    if [[ ! -f $T/filelist ]]; then
        echo -n "Creating index..."
        (\cd $T; find . -wholename ./out -prune -o -wholename ./.repo -prune -o -type f > filelist)
        echo " Done"
        echo ""
    fi
    local lines
    lines=($(\grep "$1" $T/filelist | sed -e 's/\/[^/]*$//' | sort | uniq))
    if [[ ${#lines[@]} = 0 ]]; then
        echo "Not found"
        return
    fi
    local pathname
    local choice
    if [[ ${#lines[@]} > 1 ]]; then
        while [[ -z "$pathname" ]]; do
            local index=1
            local line
            for line in ${lines[@]}; do
                printf "%6s %s\n" "[$index]" $line
                index=$(($index + 1))
            done
            echo
            echo -n "Select one: "
            unset choice
            read choice
            if [[ $choice -gt ${#lines[@]} || $choice -lt 1 ]]; then
                echo "Invalid choice"
                continue
            fi
            pathname=${lines[$(($choice-1))]}
        done
    else
        pathname=${lines[0]}
    fi
    \cd $T/$pathname
}

function minstall()
{
	make $1install $*
	echo "make package"
}

function mm() {
	local T=$(gettop)
	$T/scripts/mm.sh $T $*
}

function mclean()
{
	make $1clean $2
	echo "make clean"
}

function  get_build_var()
{
	local T=$(gettop)
	local build=$T/.buildconfig
	if [ -f $build ]; then
		. $build
		echo chip:$CHIP
		echo platform:$PLATFORM
		echo kernel:$KERNEL_VER
		echo board:$BOARD
	else
		echo "tina linux undefine buildconfig"
	fi
}

function pack()
{
	local T=$(gettop)
	local P=""
	local O=""
	local D=uart0
	local U=""
	local H=$(gettop)/staging_dir/host/bin/
	export PATH=$H:$PATH
	if [ "$1" = "-d" ]; then
		D=card0
	fi

	if [ "$T" ]; then
		if [ "x${CHIP}" = "xsun5i" ]; then
			P=$(gettop)/target/linux/sun5i/image/pack_img.sh
			O=$(gettop)/bin/sun5i
			U=package/boot/uboot-sun5i
		else
			P=$(gettop)/target/linux/sunxi/image/pack_img.sh
			O=$(gettop)/bin/sunxi
			U=package/boot/uboot-sunxi
		fi
	fi

	make $U/clean >> /dev/null
	make $U/install >> /dev/null

	if [ -x $P ]; then
		$P -r $O -c $CHIP -d $D -b $BOARD
	fi
}
function make_img_md5(){
    #$1: target image
    md5sum $1 | awk '{print $1}' > $1.md5
}
function refresh_ota_env(){
    local T=$(gettop)
    list=$T/package/base-files/files/etc/config/fstab
    for i in $list; do
        touch $i
    done
}
function clean_ota_env(){
    local T=$(gettop)
    local chip=sunxi
    [ x$CHIP = x"sun5i" ] && chip=sun5i
    local BIN_DIR=$T/bin/$chip
    local OTA_DIR=$BIN_DIR/ota

    [ -f $OTA_DIR/.config.old ] && cp $OTA_DIR/.config.old $T/.config
    refresh_ota_env
}
function make_ota_image(){
    local T=$(gettop)
    local chip=sunxi
    [ x$CHIP = x"sun5i" ] && chip=sun5i
    local BIN_DIR=$T/bin/$chip
    local OTA_DIR=$BIN_DIR/ota
    mkdir -p $OTA_DIR
    print_red "build ota package"

    #target image
    target_list="$BIN_DIR/boot.img $BIN_DIR/rootfs.img $BIN_DIR/usr.img"
    [ -n $1 ] && [ x$1 = x"--force" ] && rm -rf $target_list
    for i in $target_list; do
        if [ ! -f $i ]; then
            img=${i##*/}
            print_red "$i is not exsit! rebuild the image."
            make -j
            break
        fi
    done
    
    rm -rf $OTA_DIR/target_sys
    mkdir -p $OTA_DIR/target_sys
    cp $BIN_DIR/boot.img $OTA_DIR/target_sys/
    make_img_md5 $OTA_DIR/target_sys/boot.img
    
    cp $BIN_DIR/rootfs.img $OTA_DIR/target_sys/
    make_img_md5 $OTA_DIR/target_sys/rootfs.img
    
    #cp $BIN_DIR/usr.img $OTA_DIR/target_sys/
    #make_img_md5 $OTA_DIR/target_sys/usr.img

    rm -rf $OTA_DIR/usr_sys
    mkdir -p $OTA_DIR/usr_sys
    cp $BIN_DIR/usr.img $OTA_DIR/usr_sys/
    make_img_md5 $OTA_DIR/usr_sys/usr.img

    #upgrade image
    cp $T/.config $OTA_DIR/.config.old

    grep -v -e CONFIG_TARGET_ROOTFS_INITRAMFS  $OTA_DIR/.config.old > .config
    echo 'CONFIG_TARGET_ROOTFS_INITRAMFS=y' >> .config
    echo 'CONFIG_TARGET_AW_OTA_INITRAMFS=y' >> .config
    
    refresh_ota_env

    make V=s -j
    
    cp $OTA_DIR/.config.old $T/.config
    
    rm -rf $OTA_DIR/ramdisk_sys
    mkdir -p $OTA_DIR/ramdisk_sys

    cp $BIN_DIR/boot_initramfs.img $OTA_DIR/ramdisk_sys/
    make_img_md5 $OTA_DIR/ramdisk_sys/boot_initramfs.img


    cd $OTA_DIR && \
        tar -zcvf target_sys.tar.gz target_sys && \
        tar -zcvf ramdisk_sys.tar.gz ramdisk_sys && \
        tar -zcvf usr_sys.tar.gz usr_sys && \
        cd $T
    refresh_ota_env
    print_red "build ota packag finish!"
}

T=$(gettop)
buildconfig=$T/.buildconfig
if [ -f $buildconfig ]; then
	. $buildconfig
else
	echo "tina linux undefine buildconfig"
	echo "please run ./build.sh config"
fi

