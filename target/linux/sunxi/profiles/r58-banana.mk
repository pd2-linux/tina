#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R58-Banana-Board
	NAME:=R58 Banana Board
	PACKAGES := uboot-sunxi-R58-Banana-Board
endef

define Profile/R58-Banana-Board/Description
	Package set optimized for the R58 banana board
endef

$(eval $(call Profile,R58-Banana-Board))
