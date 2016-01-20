#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R16-Evb-Board
	NAME:=R16 Evb Board
	PACKAGES := uboot-sunxi-R16-Evb-Board
endef

define Profile/R16-Evb-Board/Description
	Package set optimized for the R16 dev board
	wifi module use ap6212
endef

$(eval $(call Profile,R16-Evb-Board))
