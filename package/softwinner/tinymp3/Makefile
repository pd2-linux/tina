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
PKG_NAME:=tinymp3
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=softwinner
  DEPENDS:=+libpthread +libmad +alsa-lib
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=tiny mp3 player for rootfs mode
endef


define Package/$(PKG_NAME)/description
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
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
		all

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
