include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=liballwinner
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

TINA_CHIP_TYPE = r16
TINA_PRODUCT_TYPE = tvbox

VIDEO_DECODE_LIB := libcdc_vd_avs.so \
                    libcdc_vd_h264.so \
                    libcdc_vd_h265.so \
                    libcdc_vd_mjpegplus.so \
                    libcdc_vd_mjpeg.so \
                    libcdc_vd_mpeg2.so \
                    libcdc_vd_mpeg4base.so \
                    libcdc_vd_mpeg4h263.so \
                    libcdc_vd_mpeg4normal.so \
                    libcdc_vd_mpeg4vp6.so \
                    libcdc_vd_vp8.so \
                    libcdc_vd_wmv3.so \
                    libcdc_vd_mpeg4dx.so
SYSTEM_LIB := libasound.so \
              libcrypto.so \
              libssl.so \
              libxml2.so \
              libz.so

ifeq ($(CONFIG_R8_LIBALLWINNER),y) 
    TINA_CHIP_TYPE = r8
endif

ifeq ($(CONFIG_R16_LIBALLWINNER),y) 
    TINA_CHIP_TYPE = r16
endif

ifeq ($(CONFIG_R58_LIBALLWINNER),y) 
    TINA_CHIP_TYPE = r58
endif

define Package/liballwinner
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.6  for softwinner
  DEPENDS:=+libpthread \
  	   +libopenssl \
  	   +libstdcpp \
  	   +zlib \
  	   +alsa-lib
endef

define Package/liballwinner/config
	source "$(SOURCE)/Config.in"
endef

define Package/liballwinner/Default
  TITLE:=liballwinner for all
  URL:=http://www.allwinner.com/
endef

define Package/liballwinner/description
	CedarX2.6
endef

define Package/tinarecorderdemo
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.6 recoder utils for softwinner
  DEPENDS:=+liballwinner
endef

define Package/tinarecorderdemo/description
	CedarX2.6 recorder
endef

define Package/tinaplayerdemo
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.6 tina player utils for softwinner
  DEPENDS:=+liballwinner
endef

define Package/tinaplayerdemo/description
	CedarX2.0 tina player
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./LIBRARY $(PKG_BUILD_DIR)/
	$(CP) -r ./awrecorder $(PKG_BUILD_DIR)/
	$(CP) -r ./xplayer $(PKG_BUILD_DIR)/
	$(CP) -r ./xmetadataretriever $(PKG_BUILD_DIR)/
	$(CP) -r ./Makefile.inc $(PKG_BUILD_DIR)/
	$(CP) -r ./Config.in $(PKG_BUILD_DIR)/
	$(CP) -r ./tinaplayer $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
	echo 
endef

define Build/Configure
	$(info **********TINA_CHIP_TYPE = $(TINA_CHIP_TYPE)********in Build/Configure)
	cd $(PKG_BUILD_DIR); ./LIBRARY/configure --tool-chain=ucgnueabi --cedarx-path=$(PKG_BUILD_DIR) --chip=$(TINA_CHIP_TYPE) --product=$(TINA_PRODUCT_TYPE) --ssl-enable=yes
	
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/LIBRARY \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all \
		install

	$(MAKE) -C $(PKG_BUILD_DIR)/tinaplayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
		
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/tinaplayerdemo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

	$(MAKE) -C $(PKG_BUILD_DIR)/demo/tinarecorderdemo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

# when compile fisished, the targets should be copy to $(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	############################################################
	
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/install/external/*.so                      $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/; rm -rf $(SYSTEM_LIB)
	#################################################################
	
	## copy demos.
ifneq ($(TINA_PRODUCT_TYPE),loudspeaker)
	-cp $(PKG_BUILD_DIR)/demo/tinarecorderdemo/tinarecorderdemo			   $(PKG_INSTALL_DIR)/usr/bin/
endif
	-cp $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo            	   $(PKG_INSTALL_DIR)/usr/bin/
	
	#####################################################################
	
	## copy header files.
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/cdx_config.h                                $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/log.h                                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/version.h                               $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## DEMUX
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/BASE/include                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/STREAM/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/PARSER/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/MUXER/include                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## CODEC
	-cp $(PKG_BUILD_DIR)/LIBRARY/libcedarc/include/*.h                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## PLAYER
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/soundControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/layerControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/player.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## xplayer
	-cp $(PKG_BUILD_DIR)/xplayer/awplayer.h                                      $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## awrecorder
	-cp $(PKG_BUILD_DIR)/awrecorder/awencoder.h                                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## tinaplayer
	-cp $(PKG_BUILD_DIR)/tinaplayer/tinaplayer.h                                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	#########################################################################################
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr
# the targets should be copy to $(1),for global symbol for other package
endef

define Package/liballwinner/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	############################################################
	# copy libraries
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/install/external/*.so                      $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(1)/usr/lib/

	cd $(1)/usr/lib/; rm -rf $(SYSTEM_LIB)

endef

define Package/tinarecorderdemo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
ifneq ($(TINA_PRODUCT_TYPE),loudspeaker)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/tinarecorderdemo/tinarecorderdemo				 $(1)/usr/bin/
endif
endef

define Package/tinaplayerdemo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo          			 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liballwinner))
$(eval $(call BuildPackage,tinarecorderdemo))
#$(eval $(call BuildPackage,tinaplayerdemo))
