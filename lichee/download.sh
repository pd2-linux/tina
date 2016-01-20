#!/bin/sh
#
# scripts/mkcommon.sh
# (c) Copyright 2013
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# James Deng <csjamesdeng@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

if [ "x$1" = "xr16" ] ; then
	if [ -d linux-3.4 ] ; then
		cd linux-3.4
		git checkout r16-v1.y
		cd -
	else
		git clone git://172.16.1.11/lichee/linux-3.4.git -b r16-v1.y
	fi

	if [ -d brandy ] ; then
		cd brandy
		git checkout r16-v1.y
		cd -
	else
		git clone git://172.16.1.11/lichee/brandy.git -b r16-v1.y
	fi

	exit $?
elif [ "x$1" = "xr8" ] ; then
	if [ -d linux-3.4 ] ; then
		cd linux-3.4
		git checkout r8-v1.y
		cd -
	else
		git clone git://172.16.1.11/lichee/linux-3.4.git -b r8-v1.y
	fi

	if [ -d brandy ] ; then
		cd brandy
		git checkout r8-v1.y
		cd -
	else
		git clone git://172.16.1.11/lichee/brandy.git -b r8-v1.y
	fi

	exit $?
fi

echo "Usage:\n./download.sh [r16]|[r8]"
