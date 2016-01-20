include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=liballwinner-audio
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

LIB_PATH := ./lib
AUDIO_LIB := $(LIB_PATH)/libad_audio.so \
			 $(LIB_PATH)/libaw_plugin.so \
			 $(LIB_PATH)/libcdx_base.so \
			 $(LIB_PATH)/libcdx_stream.so \
			 $(LIB_PATH)/libnormal_audio.so \
			 $(LIB_PATH)/libpostprocess.so \
			 $(LIB_PATH)/libadecoder.so \
			 $(LIB_PATH)/libaxx.so \
			 $(LIB_PATH)/libcdx_parser.so \
			 $(LIB_PATH)/libdxx.so \
			 $(LIB_PATH)/libplayer.so

include $(INCLUDE_DIR)/package.mk
 
define Package/liballwinner-audio
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0 audio for softwinner
  DEPENDS:=+libpthread \
	   +libopenssl \
	   +libxml2 \
	   +libstdcpp \
	   +libz \
	   +alsa-lib
endef

define Package/liballwinner-audio/Default
  TITLE:=liballwinner for audio
  URL:=http://www.allwinner.com/
endef

define Package/liballwinner-audio/description
	CedarX2.0 audio
endef

define Package/demoplayer
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0 player utils for softwinner
  DEPENDS:=+liballwinner-audio
endef

define Package/demoplayer/description
	CedarX2.0 player
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/DEMO
	$(CP) -r ./DEMO/demoPlayer $(PKG_BUILD_DIR)/DEMO
	$(CP) -r ./include/* $(PKG_BUILD_DIR)/
	$(CP) -r ./lib $(PKG_BUILD_DIR)/
	$(CP) -r config.mk $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/DEMO/demoPlayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"	\
# when compile fisished, the targets should be copy to $(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	$(CP) $(AUDIO_LIB) $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) ./include/* $(PKG_INSTALL_DIR)/usr/include/allwinner/

	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/
# the targets should be copy to $(1),for global symbol for other package
endef

define Package/liballwinner-audio/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(AUDIO_LIB) $(1)/usr/lib/ 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/DEMO/demoPlayer/demoPlayer $(1)/usr/bin/
endef

define Package/demoplayer/install
endef
$(eval $(call BuildPackage,liballwinner-audio))
$(eval $(call BuildPackage,demoplayer))
