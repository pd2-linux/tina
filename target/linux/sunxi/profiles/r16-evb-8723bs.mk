#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/R16-Evb-Board-8723BS
	NAME:=R16 Evb Board(8723BS)
	PACKAGES := uboot-sunxi-R16-Evb-Board-8723BS
endef

define Profile/R16-Evb-Board-8723BS/Description
	Package set optimized for the R16 dev board
	wifi module use rtl8723bs
endef

$(eval $(call Profile,R16-Evb-Board-8723BS))
