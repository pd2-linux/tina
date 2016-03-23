include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=liballwinner-audio
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include ./LIBRARY/config.mk
 
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

define Package/tinaplayerdemo
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=CedarX2.0 tina player utils for softwinner
  DEPENDS:=+liballwinner-audio
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
	$(CP) -r ./tinaplayer $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
	./LIBRARY/configure --tool-chain=ucgnueabi --chip=1667 --product=loudspeaker
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/LIBRARY \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

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
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	############################################################
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/BASE/libcdx_base.so               $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/STREAM/libcdx_stream.so           $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/PARSER/libcdx_parser.so           $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/MUXER/libcdx_muxer.so             $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/PLAYER/libplayer.so                     $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/MEMORY/libMemAdapter.so                 $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/awrecorder/libawrecorder.so          		   $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/xplayer/libxplayer.so                		   $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/xmetadataretriever/libxmetaretriever.so    	   $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                	   $(PKG_INSTALL_DIR)/usr/lib/
	
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/libnormal_audio.so  		 $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/libpostprocess.so  		 $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/*.so                      $(PKG_INSTALL_DIR)/usr/lib/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/libnormal_audio.so  	 		$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/libpostprocess.so  	 		$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/*.so                    		$(PKG_INSTALL_DIR)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/ve/ucgnueabi_1667/libVE.so       $(PKG_INSTALL_DIR)/usr/lib/
endif
	#################################################################
	## copy demos.
	$(CP) $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo            			 $(PKG_INSTALL_DIR)/usr/bin/
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
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/soundControl.h                   $(PKG_INSTALL_DIR)/usr/include/
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/layerControl.h                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/player.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## MEMORY
	cp $(PKG_BUILD_DIR)/LIBRARY/MEMORY/include/memoryAdapter.h                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## xplayer
	cp $(PKG_BUILD_DIR)/xplayer/awplayer.h                                      $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## awrecorder
	cp $(PKG_BUILD_DIR)/awrecorder/awencoder.h                                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## tinaplayer
	cp $(PKG_BUILD_DIR)/tinaplayer/tinaplayer.h                                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	cp $(PKG_BUILD_DIR)/tinaplayer/tinasoundcontrol.h                           $(PKG_INSTALL_DIR)/usr/include/
	#########################################################################################

	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/
# the targets should be copy to $(1),for global symbol for other package
endef

define Package/liballwinner-audio/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	############################################################
	## copy libraries
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/BASE/libcdx_base.so               $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/STREAM/libcdx_stream.so           $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/PARSER/libcdx_parser.so           $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/DEMUX/MUXER/libcdx_muxer.so             $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/PLAYER/libplayer.so                     $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/MEMORY/libMemAdapter.so                 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/awrecorder/libawrecorder.so                	    $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xplayer/libxplayer.so                		    $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xmetadataretriever/libxmetaretriever.so    	    $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                	    $(1)/usr/lib/
	
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/libnormal_audio.so  		 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/libpostprocess.so  		 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/lgnueabi/*.so                      $(1)/usr/lib/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/libnormal_audio.so  	 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/libpostprocess.so  	 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/uclgnueabi/*.so                   $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/ve/ucgnueabi_1667/libVE.so        $(1)/usr/lib/ 
endif

endef

define Package/tinaplayerdemo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/tinaplayerdemo/tinaplayerdemo            			 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liballwinner-audio))
#$(eval $(call BuildPackage,tinaplayerdemo))

