

include ../../config.mk
include ../../../Makefile.inc

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK = -Wl,--no-undefined


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/recoderdemo   ## output target.

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
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.c"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../../ \
                    -I../../DEMUX/BASE/include/ \
		            -I../../DEMUX/MUXER/include/ \
                    -I../../DEMUX/STREAM/include/ \
                    -I../../DEMUX/PARSER/include/ \
                    -I../../CODEC/VIDEO/DECODER/include  \
                    -I../../CODEC/VIDEO/ENCODER/include  \
                    -I../../CODEC/AUDIO/DECODER/include  \
                    -I../../CODEC/AUDIO/ENCODER/include  \
                    -I../../CODEC/SUBTITLE/DECODER/include  \
                    -I../../PLAYER/include/ \
                    -I../../PLUGIN/include/ \
                    -I../../../awrecorder

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -ldl

## set link flags
LoadFlags = $(CONFIG_FOR_LINK)

LoadFlags +=   -lawrecorder -lvencoder -laencoder   -lsdecoder  \
               -lcdx_base -lcdx_muxer -lcdx_stream -lcdx_base -lVE \
               -lMemAdapter -lpthread -ldl -lstdc++ -lrt 

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags += -laacenc -lmp3enc -lpcmenc
endif

#-law_plugin
LoadFlags += -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../../DEMUX/MUXER/ \
             -L../../CODEC/VIDEO/DECODER/ \
             -L../../CODEC/VIDEO/ENCODER/ \
             -L../../CODEC/AUDIO/DECODER/ \
             -L../../CODEC/AUDIO/ENCODER/ \
             -L../../CODEC/SUBTITLE/DECODER/ \
             -L../../PLAYER/ \
             -L../../VE/ \
             -L../../MEMORY/ \
             -L../../PLUGIN/manager \
             -L../../../awrecorder \
             
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabi/ 
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags += -L../../EXTERNAL/lib32/uclgnueabi/ 
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/linuxgnueabi
endif

ifeq ($(CONFIG_HAVE_ZLIB), $(OPTION_HAVE_ZLIB))
LoadFlags += -lz
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
#$(ObjectPath)/%.o:%.c
#	$(CC) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjectPath)/%.o:%.c
	$(CC) $(CompileFlags) -o $@ -c $<

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CC) -o $@ $^ $(LoadFlags) 

## set rules to generate .d files.
$(DependFilePath)/%.d:%.c
	set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
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


