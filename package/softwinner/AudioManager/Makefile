include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=AudioManager
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/AudioManager/Default
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=AudioManager
  DEPENDS:=+libpthread +alsa-lib +alsa-utils +libstdcpp
endef

define Package/AudioManager
  $(call Package/AudioManager/Default)
  MENU:=1
endef

define Package/AudioManager/description
  simple AudioManager for tina
endef

define Package/AudioManager-demo
  $(call Package/AudioManager/Default)
  DEPENDS:=AudioManager
  TITLE:=AudioManager demo  
endef

define Package/AudioManager-demo/description
  AudioManager demo for tina
endef


define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) src $(PKG_BUILD_DIR)/
	$(CP) demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	
	#just copy for demo
	$(CP) $(PKG_BUILD_DIR)/src/libaudiomanager.a $(PKG_BUILD_DIR)/demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/AudioManager/install
	$(INSTALL_DIR) $(1)/usr/lib/

endef

define Package/AudioManager/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/

endef

define Package/AudioManager-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/audiomanager_demo $(1)/usr/bin

endef

$(eval $(call BuildPackage,AudioManager-demo))
$(eval $(call BuildPackage,AudioManager))
