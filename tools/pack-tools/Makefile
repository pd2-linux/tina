# 
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pack-tools
PKG_VERSION:=2014.07.28

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_CAT:=bzcat
#HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) SHELL="$(BASH)" LDFLAGS="$(HOST_STATIC_LINKING)"
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/create_mbr/update_mbr $(STAGING_DIR_HOST)/bin/ 
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/update_checksum/genchecksum_uboot $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/update_fes1/update_fes1 $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/update_signature/signature $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/update_simg/simg $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/update_uboot/update_uboot $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/parser_img/parser_img $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/openssl_signature/sunxi_sign $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/script/script $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/update_mbr
	rm -f $(STAGING_DIR_HOST)/bin/update_boot0
	rm -f $(STAGING_DIR_HOST)/bin/genchecksum_uboot
	rm -f $(STAGING_DIR_HOST)/bin/update_fes1
	rm -f $(STAGING_DIR_HOST)/bin/signature
	rm -f $(STAGING_DIR_HOST)/bin/simg
	rm -f $(STAGING_DIR_HOST)/bin/update_uboot
	rm -f $(STAGING_DIR_HOST)/bin/parser_img
	rm -f $(STAGING_DIR_HOST)/bin/sunxi_sign
	rm -f $(STAGING_DIR_HOST)/bin/script
endef

$(eval $(call HostBuild))
