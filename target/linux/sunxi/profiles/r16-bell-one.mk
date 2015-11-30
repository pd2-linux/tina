#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R16-Bell-One-Board
	NAME:=R16 Bell One
	PACKAGES := uboot-sunxi-R16-Bell-One-Board
endef

define Profile/R16-Bell-One-Board/Description
	Package set optimized for the R16 dev board
endef

$(eval $(call Profile,R16-Bell-One-Board))
