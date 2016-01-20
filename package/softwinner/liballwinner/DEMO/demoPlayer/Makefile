
include ../../config.mk

################################################################################
## config tool chains
################################################################################
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))

# set compile tool chains to be arm-linux-gnueabihf- set.
CC    = arm-linux-gnueabihf-gcc
CPP   = arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip
AR    = arm-linux-gnueabihf-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))

# set compile tool chains to be arm-none-linux-gnueabi- set.
CC    = arm-none-linux-gnueabi-gcc
CPP   = arm-none-linux-gnueabi-g++
STRIP = arm-none-linux-gnueabi-strip
AR    = arm-none-linux-gnueabi-ar

endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))

# set compile tool chains to be arm-none-linux-gnueabi- set.
CC    ?= arm-openwrt-linux-uclibcgnueabi-gcc
CPP   ?= arm-openwrt-linux-uclibcgnueabi-g++
STRIP ?= arm-openwrt-linux-uclibcgnueabi-strip
AR    ?= arm-openwrt-linux-uclibcgnueabi-ar

endif


################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK = 


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/demoPlayer   ## output target.

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(OutputPath),wildcard($(OutputPath)))
a := $(shell mkdir -p $(OutputPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif


################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SvnPath = $(shell find ./ -type d | grep ".svn")
SourcePath := $(filter-out $(SvnPath) $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the source files.
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.cpp"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
ifneq ($(CONFIG_PRODUCT),$(OPTION_PRODUCT_LOUDSPEAKER))
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../../ \
                    -I../../DEMUX/BASE/include/ \
                    -I../../DEMUX/STREAM/include/ \
                    -I../../DEMUX/PARSER/include/ \
                    -I../../CODEC/VIDEO/DECODER/include/  \
                    -I../../CODEC/AUDIO/DECODER/include/  \
                    -I../../CODEC/SUBTITLE/DECODER/include/  \
                    -I../../PLAYER/include/                  \
                    -I../../EXTERNAL/include/libsalsa/                          
else
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../../ \
                    -I../../DEMUX/BASE/include/ \
                    -I../../DEMUX/STREAM/include/ \
                    -I../../DEMUX/PARSER/include/ \
                    -I../../CODEC/VIDEO/DECODER/  \
                    -I../../CODEC/AUDIO/DECODER/  \
                    -I../../CODEC/SUBTITLE/DECODER/  \
                    -I../../PLAYER/                  \
                    -I../../EXTERNAL/include/libsalsa/  
endif
## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -O3 -ldl

## set link flags
ifneq ($(CONFIG_PRODUCT),$(OPTION_PRODUCT_LOUDSPEAKER))
LoadFlags = $(CONFIG_FOR_LINK) -lplayer -lvdecoder -ladecoder -lnormal_audio -lad_audio -lsdecoder \
             -lcdx_parser -lcdx_stream -lcdx_base -lVE -lMemAdapter \
              -lxml2 -lz -lcrypto -lssl -lasound -lpthread -ldl -lstdc++
LoadFlags += -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../../CODEC/VIDEO/DECODER/ \
             -L../../CODEC/AUDIO/DECODER/ \
             -L../../CODEC/SUBTITLE/DECODER/ \
             -L../../PLAYER/ \
             -L../../VE/ \
             -L../../MEMORY/
else
LoadFlags = $(CONFIG_FOR_LINK) -lplayer  -ladecoder -lnormal_audio -lad_audio \
             -lcdx_parser -lcdx_stream -lcdx_base   \
              -lxml2 -lz -lcrypto -lssl -lasound -lpthread -ldl -lstdc++
LoadFlags += -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../../CODEC/AUDIO/DECODER/ \
             -L../../PLAYER/ \
             -L../../lib/ 
endif

             
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/build-by-arm-linux-gnueabihf/libxml2/lib/ \
             -L../../EXTERNAL/build-by-arm-linux-gnueabihf/zlib/lib/ \
             -L../../EXTERNAL/build-by-arm-linux-gnueabihf/openssl/lib/ 
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags += -L../../EXTERNAL/build-by-arm-none-linux-gnueabi/libxml2/lib/ \
             -L../../EXTERNAL/build-by-arm-none-linux-gnueabi/zlib/lib/ \
             -L../../EXTERNAL/build-by-arm-none-linux-gnueabi/openssl/lib/ 
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))

# set compile tool chains to be arm-none-linux-gnueabi- set.
LoadFlags += -L../../EXTERNAL/lib32/uclgnueabi/ 
    
endif



################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -f $(ObjectPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -f $(DependFilePath)/*
	-rm -rf $(BuildPath)


################################################################################
## define target dependencies.
################################################################################

## compile source files to object files.
#$(ObjectPath)/%.o:%.cpp
#	$(CPP) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
	$(CPP) $(CompileFlags) -o $@ -c $<

## link object files to the target share library.
$(Target):$(ObjectFiles)
	echo $(LoadFlags)
	$(CPP) -o $@ $^ $(LoadFlags) 

## set rules to generate .d files.
$(DependFilePath)/%.d:%.cpp
	set -e; rm -f $@; \
	$(CPP) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,\($*\)\.o[:]*,$(ObjectPath)/\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif


