include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifimanager
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/wifimanager/Default
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=Tina wifi manager
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base
endef
 
define Package/wifimanager
  $(call Package/wifimanager/Default) 
  MENU:=1
  DEFAULT:=1
endef

define Package/wifimanager-demo
  $(call Package/wifimanager/Default)
  TITLE:=Tina wifi app demo
  DEPENDS := wifimanager +wifimanager
endef

define Package/wifimanager/description
	Tina wifi manager
endef

define Package/wifimanager-demo/description
	Tina wifi app demo
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#libwifimg.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

  	#wifitest
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/wifimanager/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) ./src/wpa_supplicant.conf $(1)/etc/wifi
	$(INSTALL_DATA) ./src/wpa_supplicant_src.conf $(1)/etc/wifi
	$(INSTALL_DATA) ./src/wpa_supplicant_overlay.conf $(1)/etc/wifi
	$(INSTALL_BIN) ./wifi.init $(1)/etc/wifi/wifi
	$(INSTALL_BIN) ./udhcpc_wlan0.init $(1)/etc/wifi/udhcpc_wlan0
endef

define Package/wifimanager-demo/install
	$(INSTALL_DIR) $(1)/bin        
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_connect_ap_test $(1)/bin	
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_on_off_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_connect_chinese_ap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_add_network_test $(1)/bin
	$(INSTALL_BIN) ./wifitest.sh $(1)/etc/wifi
endef

$(eval $(call BuildPackage,wifimanager))
$(eval $(call BuildPackage,wifimanager-demo)) 
