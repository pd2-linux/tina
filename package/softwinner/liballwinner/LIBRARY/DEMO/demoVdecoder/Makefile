
include ../../config.mk

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

###############################################################################
###############################################################################
BuildPath      = ./build
OutputPath     = ./
CurrentPath     = ./
ObjCommonPath  = ./build/obj
DepCommonPath  = ./build/dep
Target         = $(OutputPath)/demoVdecoder   ## output target.

SourceFiles    := demoVdecoder.c

ObjectFiles	:= $(patsubst %.c,$(ObjCommonPath)/%.o,$(SourceFiles))
ObjectPath := $(dir $(ObjectFiles))
DependFiles	:= $(patsubst %.c,$(ObjCommonPath)/%.d,$(SourceFiles))
DependFilePath := $(dir $(DependFiles))

DemoVdecoderInstallPath := ./app
DemoVdecoderLibPath := $(DemoVdecoderInstallPath)/lib

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif
			
SourceIncludePath :=	-I../../CODEC/VIDEO/DECODER/include/ \
	                    -I../../MEMORY/include/    \
	                    -I../../VE/include/        \
	                    -I../../DEMUX/BASE/include/ \
	                    -I../../DEMUX/STREAM/include/ \
	                    -I../../DEMUX/PARSER/include/ \
	                    -I../../CODEC/SUBTITLE/DECODER/  \
	                    -I../../CODEC/AUDIO/DECODER/include/ \
	                    -I../../CODEC/SUBTITLE/DECODER/include/ \
	                    -I../../

		 
CompileFlags := $(SourceIncludePath) -O2  -Wall -fPIC -ldl -march=armv7-a	
	 
LoadFlags := -L../../MEMORY/   \
			 -L../../VE/		\
			 -L../../CODEC/VIDEO/DECODER/	\
			 -L../../DEMUX/STREAM/			\
			 -L../../DEMUX/PARSER/			\
			 -L../../DEMUX/BASE/			
			 

LoadFlags += -lvdecoder -lrv -lVE -lMemAdapter -lcdx_base -lcdx_parser -lcdx_stream -lpthread 

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/lgnueabi
endif

LoadFlags += -Wl,--no-undefined

###############################################################################
###############################################################################
.PHONY: all clean cleanall
all:$(Target)

clean:
	-rm -rf $(ObjCommonPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -rf $(DepCommonPath)/*
	-rm -rf $(DemoVdecoderInstallPath)
	-rm -rf $(BuildPath)


###############################################################################
###############################################################################
$(ObjectFiles):$(ObjCommonPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CC) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DepCommonPath)/%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjCommonPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	@echo "updating    $@"


###############################################################################
###############################################################################
	#copy libraries
	#-mkdir -p install/lib/
	#-cp DEMUX/BASE/libcdx_base.so install/lib/
install:

ifneq ($(DemoVdecoderInstallPath),wildcard($(DemoVdecoderInstallPath)))
	$(shell mkdir -p $(DemoVdecoderInstallPath))
endif
ifneq ($(DemoVdecoderLibPath),wildcard($(DemoVdecoderLibPath)))
	$(shell mkdir -p $(DemoVdecoderLibPath))
endif
	-rm -f ${DemoVdecoderLibPath}/*
	
	-cp ../../VE/*.so ${DemoVdecoderLibPath}/
	-cp ../../MEMORY/*.so ${DemoVdecoderLibPath}/
	-cp ../../CODEC/VIDEO/DECODER/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/STREAM/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/PARSER/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/BASE/*.so ${DemoVdecoderLibPath}/
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
	-cp ../../EXTERNAL/lib32/lgnueabihf/*.so ${DemoVdecoderLibPath}/
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	-cp ../../EXTERNAL/lib32/lgnueabi/*.so ${DemoVdecoderLibPath}/
endif
	-cd ${DemoVdecoderLibPath};ln -s libxml2.so libxml2.so.2;cd -	
	-chmod a+x ${DemoVdecoderLibPath}/*.so
	-cp $(Target) $(DemoVdecoderInstallPath)/
	-cp $(CurrentPath)/doc/readme.txt $(DemoVdecoderInstallPath)/
	
## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
