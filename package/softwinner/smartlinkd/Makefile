##############################################
# OpenWrt Makefile for smartlinkd program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=smartlinkd
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=softwinner
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +wifimanager
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd for Tina
  MENU:=1
endef

define Package/$(PKG_NAME)-demo
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd demo
  DEPENDS:=$(PKG_NAME)
endef

define Package/$(PKG_NAME)/description
	wifi configuration for none input platform
endef
 
define Package/$(PKG_NAME)-demo/description
	client demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	
	#compile libs and service
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		SAVE_DIR="`pwd`" \
		all

	#compile demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_DATA) src/include/smartlink_util.h $(1)/usr/include
	$(INSTALL_DATA) src/libclient/connect.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/wifi/smartlink
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsmartlinkd_client.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/setup $(1)/etc/wifi/smartlink/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd $(1)/etc/wifi/smartlink/

	$(INSTALL_BIN) ./tina/smartlinkd.init $(1)/etc/init.d/smartlinkd
	$(INSTALL_BIN) ./tina/wpa.sh $(1)/etc/wifi/smartlink/
	
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd_demo $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
