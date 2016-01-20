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

function make_ota_image(){
	local T=$(gettop)
	printf "build ota package\n"
	[ -e $T/package/utils/otabuilder/Makefile ] && 
		make -j
		make package/utils/otabuilder/clean -j
		make package/utils/otabuilder/install -j
		print_red bin/sunxi/ota_md5.tar.gz
	printf "build ota package end\n"
}

T=$(gettop)
buildconfig=$T/.buildconfig
if [ -f $buildconfig ]; then
	. $buildconfig
else
	echo "tina linux undefine buildconfig"
	echo "please run ./build.sh config"
fi

