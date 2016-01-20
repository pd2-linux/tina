#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R8-NextThing-Board
	NAME:=R8 NextThing Board
	PACKAGES := uboot-sun5i-R8-NextThing-Board
endef

define Profile/R8-NextThing-Board/Description
	Package set optimized for the R8 dev board
endef

$(eval $(call Profile,R8-NextThing-Board))
