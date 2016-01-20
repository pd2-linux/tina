include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=mali400-um
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
 
define Package/mali400-um
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=mali400 userspace libs
  DEPENDS:=+libpthread +librt +libUMP
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/mali400-um/description
	mali400 userspace libraries
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./lib/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)/usr/include/
	mkdir -p $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) ./include/* $(PKG_INSTALL_DIR)/usr/include/
	$(CP) ./lib/* $(PKG_INSTALL_DIR)/usr/lib/

	mkdir -p $(1)/usr/include
	mkdir -p $(1)/usr/lib
	$(CP) ./include/* $(1)/usr/include/
	$(CP) ./lib/* $(1)/usr/lib/
endef

define Package/mali400-um/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libEGL.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libEGL.so.1 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libEGL.so.1.4 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv1_CM.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv1_CM.so.1 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv1_CM.so.1.1 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv2.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv2.so.2 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libGLESv2.so.2.0 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libMali.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,mali400-um))
