include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=liballwinner
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include ./LIBRARY/config.mk
 
TINA_CHIP_TYPE = 1667
TINA_PRODUCT_TYPE = pad

ifeq ($(CONFIG_R8_ALL_ENDECODE),y) 
    TINA_CHIP_TYPE = 1625
    TINA_PRODUCT_TYPE = pad
endif

ifeq ($(CONFIG_R8_ONLY_AUDIO_DECODE),y) 
    TINA_CHIP_TYPE = 1625
    TINA_PRODUCT_TYPE = loudspeaker
endif

ifeq ($(CONFIG_R16_ALL_ENDECODE),y) 
    TINA_CHIP_TYPE = 1667
    TINA_PRODUCT_TYPE = pad
endif

ifeq ($(CONFIG_R16_ONLY_AUDIO_DECODE),y) 
    TINA_CHIP_TYPE = 1667
    TINA_PRODUCT_TYPE = loudspeaker
endif

ifeq ($(CONFIG_R58_ALL_ENDECODE),y) 
    TINA_CHIP_TYPE = 1673
    TINA_PRODUCT_TYPE = pad
endif

ifeq ($(CONFIG_R58_ONLY_AUDIO_DECODE),y) 
    TINA_CHIP_TYPE = 1673
    TINA_PRODUCT_TYPE=loudspeaker
endif

define Package/liballwinner
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0  for softwinner
  DEPENDS:=+libpthread \
  	   +libopenssl \
  	   +libxml2 \
  	   +libstdcpp \
  	   +libz \
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
	CedarX2.0 
endef

define Package/recoderdemo
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0 recoder utils for softwinner
  DEPENDS:=+liballwinner
endef

define Package/recoderdemo/description
	CedarX2.0 recoder
endef

define Package/tinaplayerdemo
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0 tina player utils for softwinner
  DEPENDS:=+liballwinner
endef

define Package/tinaplayerdemo/description
	CedarX2.0 tina player
endef

define Build/Prepare
	$(warning ***********Build/Prepare***************)
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
	$(warning ***********Build/Configure***************)
	$(warning **********R8_ALL_ENDECODE = $(CONFIG_R8_ALL_ENDECODE)********in Build/Configure)
	$(warning **********R8_ONLY_AUDIO_DECODE = $(CONFIG_R8_ONLY_AUDIO_DECODE)********in Build/Configure)
	$(warning **********R16_ALL_ENDECODE = $(CONFIG_R16_ALL_ENDECODE)********in Build/Configure)
	$(warning **********R16_ONLY_AUDIO_DECODE = $(CONFIG_R16_ONLY_AUDIO_DECODE)********in Build/Configure)
	$(warning **********R58_ALL_ENDECODE = $(CONFIG_R58_ALL_ENDECODE)********in Build/Configure)
	$(warning **********R58_ONLY_AUDIO_DECODE = $(CONFIG_R58_ONLY_AUDIO_DECODE)********in Build/Configure)
	$(warning **********TINA_CHIP_TYPE = $(TINA_CHIP_TYPE)********in Build/Configure)
	$(warning **********TINA_PRODUCT_TYPE = $(TINA_PRODUCT_TYPE)********in Build/Configure)
	cd $(PKG_BUILD_DIR); ./LIBRARY/configure --tool-chain=ucgnueabi --chip=$(TINA_CHIP_TYPE) --product=$(TINA_PRODUCT_TYPE)
	
endef

define Build/Compile
	$(warning ***********Build/Compile***************)
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

# when compile fisished, the targets should be copy to $(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	$(warning ***********Build/InstallDev***************)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	############################################################
	
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(PKG_INSTALL_DIR)/usr/lib/

	#################################################################
	
	## copy demos.
ifneq ($(TINA_PRODUCT_TYPE),loudspeaker)
	-cp $(PKG_BUILD_DIR)/LIBRARY/install/bin/recoderdemo            		   $(PKG_INSTALL_DIR)/usr/bin/
endif
	-cp $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo            	   $(PKG_INSTALL_DIR)/usr/bin/
	
	#####################################################################
	
	## copy header files.
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/config.h                                $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/log.h                                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/version.h                               $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## DEMUX
	cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/BASE/include                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/STREAM/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/PARSER/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/MUXER/include                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## CODEC
	cp $(PKG_BUILD_DIR)/LIBRARY/CODEC/VIDEO/DECODER/include/vdecoder.h          $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/LIBRARY/CODEC/AUDIO/DECODER/include/adecoder.h          $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/LIBRARY/CODEC/SUBTITLE/DECODER/include/sdecoder.h       $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## PLAYER
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/soundControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/layerControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/player.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## MEMORY
	cp $(PKG_BUILD_DIR)/LIBRARY/MEMORY/include/memoryAdapter.h                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## xplayer
	cp $(PKG_BUILD_DIR)/xplayer/awplayer.h                                      $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## awrecorder
	cp $(PKG_BUILD_DIR)/awrecorder/awencoder.h                                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## tinaplayer
	cp $(PKG_BUILD_DIR)/tinaplayer/tinaplayer.h                                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	#########################################################################################

	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/
# the targets should be copy to $(1),for global symbol for other package
endef

define Package/liballwinner/install
	$(warning ***********Package/liballwinner/install***************)
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	############################################################
	## copy libraries
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(1)/usr/lib/
endef

define Package/recoderdemo/install
	$(warning ***********Package/recoderdemo/install***************)
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
ifneq ($(TINA_PRODUCT_TYPE),loudspeaker)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/DEMO/recoderdemo/recoderdemo            		 $(1)/usr/bin/
endif
endef

define Package/tinaplayerdemo/install
	$(warning ***********Package/tinaplayerdemo/install***************)
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo          			 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liballwinner))
$(eval $(call BuildPackage,recoderdemo))
#$(eval $(call BuildPackage,tinaplayerdemo))

