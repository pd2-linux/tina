# scripts/mkcmd.sh
#
# (c) Copyright 2013
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# James Deng <csjamesdeng@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Notice:
#   1. This script muse source at the top directory of lichee.

cpu_cores=`cat /proc/cpuinfo | grep "processor" | wc -l`
if [ ${cpu_cores} -le 8 ] ; then
	JLEVEL=${cpu_cores}
else
	JLEVEL=`expr ${cpu_cores} / 2`
fi

export JLEVEL

function mk_error()
{
	echo -e "\033[47;31mERROR: $*\033[0m"
}

function mk_warn()
{
	echo -e "\033[47;34mWARN: $*\033[0m"
}

function mk_info()
{
	echo -e "\033[47;30mINFO: $*\033[0m"
}

# define importance variable
TOP_DIR=`pwd`
SUNXI_BOOT_DIR=${TOP_DIR}/package/boot/uboot-sunxi
SUN5I_BOOT_DIR=${TOP_DIR}/package/boot/uboot-sun5i
KERNEL_DIR=${TOP_DIR}/lichee/linux-3.4

# make surce at the top directory of lichee
if [ ! -d ${KERNNEL_DIR} ] ; then
	mk_error "You are not at the top directory of openwrt."
	mk_error "Please changes to that directory."
	exit 1
fi

# export importance variable
export TOP_DIR
export SUNXI_BOOT_DIR
export SUN5I_BOOT_DIR
export KERNEL_DIR

platforms=(
	"dragonboard"
	"tina"
)

function check_env()
{
	if [ -z "${CHIP}" -o \
		-z "${PLATFORM}" -o \
		-z "${BOARD}" ] ; then
		mk_error "run './build.sh config' setup env"
		exit 1
	fi
}

function init_defconf()
{
	local pattern
	local defconf

	check_env

	pattern="${CHIP}_${PLATFORM}_${BOARD}"
	defconf=`awk '$1=="'$pattern'" {print $2,$3}' scripts/mkrule`
	if [ -n "${defconf}" ] ; then
		export WRT_DEFCONF=`echo ${defconf} | awk '{print $1}'`
		export KERNEL_DEFCONF=`echo ${defconf} | awk '{print $2}'`
	else
		pattern="${CHIP}_${PLATFORM}"
		defconf=`awk '$1=="'$pattern'" {print $2,$3}' scripts/mkrule`
		if [ -n "${defconf}" ] ; then
			export WRT_DEFCONF=`echo ${defconf} | awk '{print $1}'`
			export KERNEL_DEFCONF=`echo ${defconf} | awk '{print $2}'`
		fi
	fi

}

function prepare_defconf()
{
	./scripts/feeds install -a
	cp config/${WRT_DEFCONF} .config
	#if [ "x${CHIP} = "xsun5i" ]
	#	cp config/${KERNEL_DEFCONF} target/linux/sun5i/config-3.4
	#else
	#	cp config/${KERNEL_DEFCONF} target/linux/sunxi/config-3.4
	#fi
}

function mkrelease()
{
	cp .config config/${WRT_DEFCONF} 
	#cp target/linux/sunxi/config-3.4 config/${KERNEL_DEFCONF}
}

function init_chips()
{
	local chip=$1
	local cnt=0
	local ret=1

	for chipdir in ${SUN5I_BOOT_DIR}/bootloader/sun* ; do
		chips[$cnt]=`basename $chipdir`
		if [ "x${chips[$cnt]}" = "x${chip}" ] ; then
			ret=0
			export CHIP=${chip}
		fi
		((cnt+=1))
	done

	for chipdir in ${SUNXI_BOOT_DIR}/bootloader/sun* ; do
		chips[$cnt]=`basename $chipdir`
		if [ "x${chips[$cnt]}" = "x${chip}" ] ; then
			ret=0
			export CHIP=${chip}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_platforms()
{
	local cnt=0
	local ret=1

	for platform in ${platforms[@]} ; do
		if [ "x${platform}" = "x$1" ] ; then
			ret=0
			export PLATFORM=${platform}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_kern_ver()
{
	local kern_ver=$1
	local cnt=0
	local ret=1

	for kern_dir in ${TOP_DIR}/target/linux/sunxi/linux-* ; do
		kern_vers[$cnt]=`basename $kern_dir`
		if [ "x${kern_vers[$cnt]}" = "x${kern_ver}" ] ; then
			ret=0
			export KERN_VER=${kern_ver}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_boards()
{
	local chip=$1
	local platform=$2
	local kern_ver=$3
	local board=$4
	local cnt=0
	local ret=1

	for boarddir in ${BOOT_DIR}/bootloader/${chip}/configs/* ; do
		boards[$cnt]=`basename $boarddir`
		if [ "x${boards[$cnt]}" = "x${board}" ] ; then
			ret=0
			export BOARD=${board}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function select_chip()
{
	local cnt=0
	local choice
	local call=$1

	printf "All available chips:\n"

	for chipdir in ${SUN5I_BOOT_DIR}/bootloader/* ; do
		chips[$cnt]=`basename $chipdir`
		if [ "x${chips[$cnt]}" = "xtools" -o \
			"x${chips[$cnt]}" = "xboot-resource" ] ; then
			continue
		fi
		printf "%4d. %s\n" $cnt ${chips[$cnt]}
		((cnt+=1))
	done

	for chipdir in ${SUNXI_BOOT_DIR}/bootloader/* ; do
		chips[$cnt]=`basename $chipdir`
		if [ "x${chips[$cnt]}" = "xtools" -o \
			"x${chips[$cnt]}" = "xboot-resource" ] ; then
			continue
		fi
		printf "%4d. %s\n" $cnt ${chips[$cnt]}
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi
		export CHIP="${chips[$choice]}"
		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export CHIP="${chips[$choice]}"
				echo "export CHIP=${chips[$choice]}" >> .buildconfig
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_platform()
{
	local cnt=0
	local choice
	local call=$1

	select_chip

	printf "All available platforms:\n"
	for platform in ${platforms[@]} ; do
		printf "%4d. %s\n" $cnt $platform
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export PLATFORM="${platforms[$choice]}"
				echo "export PLATFORM=${platforms[$choice]}" >> .buildconfig
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_kern_ver()
{
	local cnt=0
	local choice

	select_platform
	printf "All available kernel:\n"
	for kern_dir in ${TOP_DIR}/lichee/linux-* ; do
		kern_vers[$cnt]=`basename $kern_dir`
		printf "%4d. %s\n" $cnt ${kern_vers[$cnt]}
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export KERNEL_VER="${kern_vers[$choice]}"
				echo "export KERNEL_VER=${kern_vers[$choice]}" >> .buildconfig
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_board()
{
	local cnt=0
	local choice

	select_kern_ver

	printf "All available boards:\n"
	for boarddir in ${SUN5I_BOOT_DIR}/bootloader/${CHIP}/configs/* ; do
		boards[$cnt]=`basename $boarddir`
		
		if [ "x${boards[$cnt]}" = "x*" ] ; then
			break
		fi

		if [ "x${boards[$cnt]}" = "xdefault" ] ; then
			continue
		fi

		printf "%4d. %s\n" $cnt ${boards[$cnt]}
		((cnt+=1))
	done

	for boarddir in ${SUNXI_BOOT_DIR}/bootloader/${CHIP}/configs/* ; do
		boards[$cnt]=`basename $boarddir`
		if [ "x${boards[$cnt]}" = "x*" ] ; then
			break
		fi

		if [ "x${boards[$cnt]}" = "xdefault" ] ; then
			continue
		fi
		printf "%4d. %s\n" $cnt ${boards[$cnt]}
		((cnt+=1))
	done


	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export BOARD="${boards[$choice]}"
				echo "export BOARD=${boards[$choice]}" >> .buildconfig
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function mkboot()
{
	mk_info "build boot ..."
	mk_info "build boot OK."
}

function mkrootfs()
{
	mk_info "build rootfs ..."
	mk_info "build rootfs OK."
}

function mkopenwrt()
{
	mk_info "build openwrt ..."
	make -j$JLEVEL $@
	mk_info "build openwrt OK."
}

function mktina()
{

	mk_info "----------------------------------------"
	mk_info "build tina ..."
	mk_info "chip: $CHIP"
	mk_info "platform: $PLATFORM"
	mk_info "kernel: $KERNEL_VER"
	mk_info "board: $BOARD"
	mk_info "output: bin/sunxi/"
	mk_info "----------------------------------------"

	check_env

	mkopenwrt $@
	[ $? -ne 0 ] && return 1

	mk_info "----------------------------------------"
	mk_info "build tina OK."
	mk_info "----------------------------------------"
}

function mkclean()
{
	mk_info "clean tina"
	make clean
}

function mkdistclean()
{
	mk_info "distclean tina ..."
	make distclean
}

function mkpack()
{
	mk_info "packing firmware ..."

	check_env

	(cd target/linux/sunxi/image && \
		./pack_img.sh -b bin/sunxi -c ${CHIP} -b ${BOARD} $@)
}

function mkhelp()
{
	printf "
	mkscript - lichee build script
	<version>: 1.0.0
	<author >: james

	<command>:
	mkboot      build boot
	mkopenwrt    build total lichee

	mkclean     clean current board output
	mkdistclean clean entires output

	mkpack      pack firmware for lichee

	mkhelp      show this message

	"
}

