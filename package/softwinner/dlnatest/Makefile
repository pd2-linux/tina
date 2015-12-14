include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dlnatest
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
 
define Package/dlnatest
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=dlnatest
  DEPENDS:=+liballwinner-audio
endef

define Package/dlnatest/Default
  TITLE:=liballwinner for audio
  URL:=http://www.allwinner.com/
endef

define Package/dlnatest/description
  dlnatest`	
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
endef

define Package/dlnatest/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DATA) ./*.so $(1)/usr/lib/ 
	$(INSTALL_BIN) ./dlnatest $(1)/usr/bin/
endef

$(eval $(call BuildPackage,dlnatest))
