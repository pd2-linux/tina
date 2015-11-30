#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mali400-km
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mali400-km
  SUBMENU:=Other modules
  TITLE:=mali400-km400 driver
  DEPENDS:= +kmod-sunxi-disp
  FILES:=$(LINUX_DIR)/modules/mali/DX910-SW-99002-r4p0-00rel0/driver/src/devicedrv/mali/mali.ko
  AUTOLOAD:=$(call AutoLoad,50,mali)
  KCONFIG:=
endef

define KernelPackage/mali400-km/description
 Kernel module for mali400-km support.
endef


MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(LINUX_DIR)/modules/mali/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)/modules/mali" \
	LICHEE_KDIR=$(LINUX_DIR) \
		$(MAKE_OPTS) build
endef

$(eval $(call KernelPackage,mali400-km))
