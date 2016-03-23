

include ../../config.mk
include ../../../Makefile.inc

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK = 


################################################################################

BuildPath      = ./build
ObjCommonPath     = $(BuildPath)/obj
OutputPath     = ./
DepCommonPath = $(BuildPath)/dep
Target         = $(OutputPath)/libcdx_muxer.so   ## output target.

################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SvnPath = $(shell find ./ -type d | grep ".svn")
testPath = $(shell find ./ -type d | grep "test")
SourcePath := $(filter-out $(SvnPath) $(testPath) , $(SourcePath))             

                       
VPATH := $(SourcePath)

## set the source files.
SourceFiles  = $(foreach dirTmp,$(SourcePath),$(shell find $(dirTmp) -maxdepth 1 -name "*.c"))

## set the object files.
ObjectFiles = $(patsubst ./%.c, $(ObjCommonPath)/%.o ,$(SourceFiles))
ObjectPath := $(dir $(ObjectFiles))

## set the dependency files.
DependFiles = $(patsubst ./%.c, $(DepCommonPath)/%.d ,$(SourceFiles))
DependFilePath := $(dir $(DependFiles))

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif

################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../BASE/include \
                    -I../STREAM/include \
                    -I../PARSER/include          \
					-I../../          \
                    -I../../CODEC/VIDEO/DECODER/include/ \
					-I../../CODEC/VIDEO/ENCODER/include/ \
                    -I../../CODEC/AUDIO/DECODER/include/ \
					-I../../CODEC/AUDIO/ENCODER/include/ \
                    -I../../CODEC/SUBTITLE/DECODER/include/ \
                    -I../../PLAYER/include/ \
                    -I../../EXTERNAL/include/zlib/ \
                    -I../../EXTERNAL/include/openssl/ \
                    -I../../EXTERNAL/include/libxml \
                    -I../../EXTERNAL/include/zlib  \
                    -I../../EXTERNAL/include
                    
## set compile flags
ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1689))
	CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath)  -O2 -fPIC -ldl -march=armv8-a 
else
	ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_1663))
		CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O2 -fPIC -ldl -march=armv5te	
	else
		CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O2 -fPIC -ldl -march=armv7-a
	endif
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
#CompileFlags += -mfloat-abi=softfp -mfpu=neon 
endif

## set link flags
#LoadFlags = $(CONFIG_FOR_LINK) -lGetAudio_format -ldl -shared
LoadFlags = $(CONFIG_FOR_LINK) -ldl -shared -lpthread
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/lgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/uclgnueabi
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/linuxgnueabi
endif

LoadFlags += -L../BASE/
LoadFlags += -lcdx_base

#LoadFlags += -L../STREAM/
#LoadFlags += -lcdx_stream
#LoadFlags += -Wl,--no-undefined



################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -rf $(ObjCommonPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -rf $(DepCommonPath)/*
	-rm -rf $(BuildPath)


################################################################################
## define target dependencies.
################################################################################

## compile source files to object files.
#$(ObjectPath)/%.o:%.c
#	$(CC) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjCommonPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CC) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DependFiles):$(DepCommonPath)/%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjCommonPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	@echo "updating    $@"

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif


