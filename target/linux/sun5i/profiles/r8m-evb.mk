#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R8M-Evb-Board
	NAME:=R8M Evb Board
	PACKAGES := uboot-sun5i-R8M-Evb-Board
endef

define Profile/R8M-Evb-Board/Description
	Package set optimized for the R8M dev board
endef

$(eval $(call Profile,R8M-Evb-Board))
