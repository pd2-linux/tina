#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sun5i-nand
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sun5i-nand
  SUBMENU:=Other modules
  TITLE:=sun5i nand flash driver
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/nand.ko
  AUTOLOAD:=$(call AutoLoad,50,nand)
  KCONFIG:=
endef

define KernelPackage/sun5i-nand/description
 Kernel module for nand flash support.
endef


MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./nand/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,sun5i-nand))
