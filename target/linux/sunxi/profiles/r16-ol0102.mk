#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R16-Ol0102-Board
	NAME:=R16 Ol0102 Board
	PACKAGES := uboot-sunxi-R16-Ol0102-Board
endef

define Profile/R16-Ol0102-Board/Description
	Package set optimized for the R16 Ol0102 board
	wifi module use ap6212
endef

$(eval $(call Profile,R16-Ol0102-Board))
