include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=liballwinner-base
PKG_VERSION:=1
PKG_RELEASE:=1

HAVE_SRC := no

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
 
define Package/liballwinner-base
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=base
endef

define Package/liballwinner-base/Default
endef

define Package/liballwinner-base/description
endef


define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/

ifeq ($(HAVE_SRC),yes)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endif

endef

define Build/Configure
endef

define Build/Compile

ifeq ($(HAVE_SRC),yes)
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all 
endif

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) tina_log.h $(1)/usr/include/
	$(INSTALL_DATA) locker.h $(1)/usr/include/
	$(INSTALL_DATA) taskbase.h $(1)/usr/include/
	$(INSTALL_DATA) thread.h $(1)/usr/include/


ifeq ($(HAVE_SRC),yes)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libawbase.so $(1)/usr/lib
else
	$(INSTALL_BIN) ./libawbase.so $(PKG_INSTALL_DIR)/usr/lib/libawbase.so
	$(INSTALL_BIN) ./libawbase.so $(1)/usr/lib
endif

endef

define Package/liballwinner-base/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib

ifeq ($(HAVE_SRC),yes)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libawbase.so $(1)/usr/lib/
	mkdir -p ./release
	cp $(PKG_INSTALL_DIR)/usr/lib/libawbase.so ./release
	cp Makefile ./release/Makefile_release
	sed -i s/"HAVE_SRC := no"/"HAVE_SRC := no"/g ./release/Makefile_release
else
	$(INSTALL_BIN) ./libawbase.so $(1)/usr/lib/
endif

endef

$(eval $(call BuildPackage,liballwinner-base))
