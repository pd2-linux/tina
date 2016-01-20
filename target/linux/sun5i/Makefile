#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=sun5i
BOARDNAME:=Allwinner R8 Support
FEATURES:=fpu usb ext4 rtc
CPU_TYPE:=cortex-a8
CPU_SUBTYPE:=neon
MAINTAINER:=Trace Wong <wangyaliang@allwinnertech.com>

KERNEL_PATCHVER:=3.4
KERNELNAME:=zImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += 

$(eval $(call BuildTarget))
