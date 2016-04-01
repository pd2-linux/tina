
include config.mk
include ../Makefile.inc

all:
	rm VE -f
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1663))
	ln -s ./EXTERNAL/lib32/ve/gnueabi_1663/ VE
endif
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1667))
	ln -s ./EXTERNAL/lib32/ve/ucgnueabi_1667/ VE
endif
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1673))
		ln -s ./EXTERNAL/lib32/ve/ucgnueabi_1673/ VE
endif
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1651))
		ln -s ./EXTERNAL/lib32/ve/linuxgnueabi_1651/ VE
endif
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1625))
		ln -s ./EXTERNAL/lib32/ve/linuxgnueabi_1625/ VE
endif
endif

	make -C MEMORY/
	make -C DEMUX/
	make -C PLAYER/
	make -C ../awrecorder/
	make -C ../xplayer/
	make -C ../xmetadataretriever/
	make -C DEMO/
	

clean:
	rm VE -f
	make clean -C MEMORY/
	make clean -C DEMUX/
	make clean -C PLAYER/
	make clean -C DEMO/
	make clean -C ../awrecorder/
	make clean -C ../xplayer/
	make clean -C ../xmetadataretriever/
	

cleanall:
	rm VE -f
	make cleanall -C MEMORY/
	make cleanall -C DEMUX/
	make cleanall -C PLAYER/
	make cleanall -C DEMO/
	make cleanall -C ../awrecorder/
	make cleanall -C ../xplayer/
	make cleanall -C ../xmetadataretriever/
	
	-rm -rf install


install:
    ############################################################
	## copy libraries
	-mkdir -p install/lib/
	-cp DEMUX/BASE/libcdx_base.so               install/lib/
	-cp DEMUX/STREAM/libcdx_stream.so           install/lib/
	-cp DEMUX/PARSER/libcdx_parser.so           install/lib/
	-cp DEMUX/MUXER/libcdx_muxer.so             install/lib/
	-cp CODEC/VIDEO/DECODER/libvdecoder.so      install/lib/
	-cp CODEC/VIDEO/ENCODER/libvencoder.so      install/lib/
	-cp CODEC/AUDIO/DECODER/libadecoder.so      install/lib/
	-cp CODEC/AUDIO/ENCODER/libaencoder.so      install/lib/
	-cp CODEC/SUBTITLE/DECODER/libsdecoder.so   install/lib/
	-cp PLAYER/libplayer.so                     install/lib/
	-cp VE/libVE.so                             install/lib/
	-cp MEMORY/libMemAdapter.so                 install/lib/
	-cp ../awrecorder/libawrecorder.so          install/lib/
	-cp ../xplayer/libxplayer.so                install/lib/
	-cp ../xmetadataretriever/libxmetaretriever.so    install/lib/
	-cp PLUGIN/manager/libaw_plugin.so                install/lib/
	
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	-cp EXTERNAL/lib32/lgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabi/libpostprocess.so  install/lib/
	
	
	-mkdir -p install/external/
	-cp EXTERNAL/lib32/lgnueabi/*.so                  install/lib/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
	-cp EXTERNAL/lib32/uclgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/uclgnueabi/libpostprocess.so  install/lib/
	
	-mkdir -p install/external/
	-cp EXTERNAL/lib32/uclgnueabi/*.so                  install/lib/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
	-cp EXTERNAL/lib32/linuxgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/libpostprocess.so  install/lib/
	
	-mkdir -p install/external/
	-cp EXTERNAL/lib32/linuxgnueabi/*.so                  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/*.so                  install/external/
endif
	#################################################################
	## copy demos.
	-mkdir -p install/bin/
	-cp DEMO/demoPlayer/demoPlayer             install/bin/
	-cp DEMO/recoderdemo/recoderdemo           install/bin/
	-cp DEMO/xplayerdemo/playerdemo            install/bin/
	-cp DEMO/xmetademo/retriverdemo            install/bin/
	-cp DEMO/muxtest/muxerdemo                 install/bin/
	-cp DEMO/jpegdemo/jpegdemo                 install/bin/
	#####################################################################
	## copy header files.
	-mkdir -p install/include/
	-cp config.h                                install/include/
	-cp log.h                                   install/include/
	-cp version.h                               install/include/
	## DEMUX
	-mkdir -p install/include/DEMUX/BASE/include
	-cp -r DEMUX/BASE/include                   install/include/DEMUX/BASE/include
	-mkdir -p install/include/DEMUX/STREAM/include
	-cp -r DEMUX/STREAM/include                 install/include/DEMUX/STREAM/include
	-mkdir -p install/include/DEMUX/PARSER/include
	-cp -r DEMUX/PARSER/include                 install/include/DEMUX/PARSER/include
	-mkdir -p install/include/DEMUX/MUXER/include
	-cp -r DEMUX/MUXER/include                 install/include/DEMUX/MUXER/include
	## CODEC
	-mkdir -p install/include/CODEC/VIDEO/DECODER/include
	-cp CODEC/VIDEO/DECODER/include/vdecoder.h          install/include/CODEC/VIDEO/DECODER/include
	-mkdir -p install/include/CODEC/AUDIO/DECODER/include
	-cp CODEC/AUDIO/DECODER/include/adecoder.h          install/include/CODEC/AUDIO/DECODER/include
	-mkdir -p install/include/CODEC/SUBTITLE/DECODER/include
	-cp CODEC/SUBTITLE/DECODER/include/sdecoder.h       install/include/CODEC/SUBTITLE/DECODER/include
	## PLAYER
	-mkdir -p install/include/PLAYER/include
	-cp PLAYER/include/player.h                         install/include/PLAYER/include
	## MEMORY
	-mkdir -p install/include/MEMORY/include
	-cp MEMORY/include/memoryAdapter.h                  install/include/MEMORY/include
	#########################################################################################
	## copy crack lib
	-mkdir -p install/crack

	$(STRIP) install/lib/*
	$(STRIP) install/bin/*

.PHONY: all clean cleanall install

