#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R16-0la100-Board
	NAME:=R16 0la100 Board
	PACKAGES := uboot-sunxi-R16-0la100-Board
endef

define Profile/R16-0la100-Board/Description
	Package set optimized for the R16 0la100 board
	wifi module use ap6212
endef

$(eval $(call Profile,R16-0la100-Board))
