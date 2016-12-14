#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-firmware
PKG_SOURCE_VERSION:=f8c22c692bdee57a20b092e647464ff6176df3ed
PKG_VERSION:=2014-03-16-$(PKG_SOURCE_VERSION)
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
PKG_MIRROR_MD5SUM:=ca4d289ad9380471cae376fc7dd3660a

PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/r8169-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=http://git.kernel.org/cgit/linux/kernel/git/firmware/linux-firmware.git
  TITLE:=RealTek r8169 firmware
endef

define Build/Compile

endef

define Package/r8169-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtl_nic
	$(CP) \
		$(PKG_BUILD_DIR)/rtl_nic/* \
		$(1)/lib/firmware/rtl_nic
endef

$(eval $(call BuildPackage,r8169-firmware))

define Package/r8188eu-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=http://git.kernel.org/cgit/linux/kernel/git/firmware/linux-firmware.git
  TITLE:=RealTek r8188eu firmware
endef

define Build/Compile

endef

define Package/r8188eu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtlwifi
	$(CP) \
		$(PKG_BUILD_DIR)/rtlwifi/rtl8188eufw.bin \
		$(1)/lib/firmware/rtlwifi
endef

$(eval $(call BuildPackage,r8188eu-firmware))


include $(INCLUDE_DIR)/package.mk
define Package/ap6212-firmware
  SECTION:=firmware    
  CATEGORY:=Firmware
  TITLE:=Broadcom AP6212 firmware
endef

define Build/Compile

endef

define Package/ap6212-firmware/install
	mkdir -p $(1)/lib/firmware
	$(CP) \
		$(TOPDIR)/package/firmware/ap6212/*  \
		$(1)/lib/firmware/
endef

$(eval $(call BuildPackage,ap6212-firmware))

define Package/ap6212a-firmware
  SECTION:=firmware    
  CATEGORY:=Firmware
  TITLE:=Broadcom AP6212A firmware
endef

define Build/Compile

endef

define Package/ap6212a-firmware/install
	mkdir -p $(1)/lib/firmware
	$(CP) \
		$(TOPDIR)/package/firmware/ap6212a/*  \
		$(1)/lib/firmware/
endef

$(eval $(call BuildPackage,ap6212a-firmware))

define Package/esp8089-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=esp8089 firmware
endef

define Package/esp8089-firmware/install
	mkdir -p $(1)/lib/firmware/esp8089
	$(CP) \
		$(TOPDIR)/package/firmware/esp8089/*  \
		$(1)/lib/firmware/esp8089/
endef

$(eval $(call BuildPackage,esp8089-firmware))
