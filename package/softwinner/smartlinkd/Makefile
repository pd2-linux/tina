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
HAVE_SOURCE := no

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=softwinner
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base
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

	mkdir -p $(PKG_BUILD_DIR)

ifeq ($(HAVE_SOURCE),yes)
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/src/
else
	$(CP) -r * $(PKG_BUILD_DIR)/
endif

endef

define Build/Configure
endef

define Build/Compile
	
ifeq ($(HAVE_SOURCE),yes)

	#compile realtek rtw_ak
	#$(MAKE) -C $(PKG_BUILD_DIR)/src/realtek/rtw_ak_r14202 \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
	
	#compile broadcom easysetup
	$(MAKE) -C $(PKG_BUILD_DIR)/src/broadcom/EasySetupTarget/jni \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
	
	#copy target
	$(CP) -r $(PKG_BUILD_DIR)/src/broadcom/EasySetupTarget/jni/setup $(PKG_BUILD_DIR)/
	#$(CP) -r $(PKG_BUILD_DIR)/src/realtek/rtw_ak_r14202/rtw_ak $(PKG_BUILD_DIR)/
	
	#compile client
	$(MAKE) -C $(PKG_BUILD_DIR)/src/client \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
	
	#compile core
	$(MAKE) -C $(PKG_BUILD_DIR)/src/server \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)"

	#copy target
	$(CP) -r $(PKG_BUILD_DIR)/src/server/smartlinkd $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/src/client/client $(PKG_BUILD_DIR)/
else

	#compile client
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endif
	
	
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DIR) $(1)/etc/init.d


ifeq ($(HAVE_SOURCE),yes)
	
	$(INSTALL_BIN) ./tina/wpa.sh $(1)/etc/wifi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setup $(1)/etc/wifi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client $(1)/etc/wifi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smartlinkd $(1)/etc/wifi/
	$(INSTALL_BIN) ./tina/smartlinkd.init $(1)/etc/init.d/smartlinkd
	
	rm -rf ./release
	mkdir -p ./release
	$(CP) Makefile ./release/
	sed -i s/"HAVE_SOURCE := no"/"HAVE_SOURCE := no"/g ./release/Makefile
	mv ./release/Makefile ./release/Makefile_release
	
	#release bin 
	$(CP) $(PKG_BUILD_DIR)/setup ./release/
	$(CP) $(PKG_BUILD_DIR)/smartlinkd ./release/
	$(CP) ./tina/wpa.sh ./release/
	$(CP) -r $(PKG_BUILD_DIR)/src/client_base ./release/
	$(CP) -r $(PKG_BUILD_DIR)/src/include ./release/
	$(CP) -r $(PKG_BUILD_DIR)/src/client ./release/demo
	$(CP) ./tina/smartlinkd.init ./release/
else
	$(INSTALL_BIN) wpa.sh $(1)/etc/wifi/
	$(INSTALL_BIN) setup $(1)/etc/wifi/
	$(INSTALL_BIN) smartlinkd $(1)/etc/wifi/
	$(INSTALL_BIN) smartlinkd.init $(1)/etc/init.d/smartlinkd
endif

endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(HAVE_SOURCE),yes)
	$(CP) -r $(PKG_BUILD_DIR)/src/client/client $(1)/usr/bin
else
	$(CP) -r $(PKG_BUILD_DIR)/demo/client $(1)/usr/bin
endif

endef
$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
