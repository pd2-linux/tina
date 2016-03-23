LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(LOCAL_PATH)/../config.mk

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = 
CONFIG_FOR_LINK = 

LOCAL_CFLAGS += $(CONFIG_FOR_COMPILE)
LOCAL_MODULE_TAGS := optional

################################################################################
## set the source files
################################################################################
## set the source path.
SourcePath = $(LOCAL_PATH)
ifeq ($(CONFIG_MEMORY_DRIVER),$(OPTION_MEMORY_DRIVER_ION))
SourcePath += $(LOCAL_PATH)/mem_alloc_ion
endif
ifeq ($(CONFIG_MEMORY_DRIVER),$(OPTION_MEMORY_DRIVER_SUNXIMEM))
SourcePath += $(LOCAL_PATH)/mem_alloc_sunxi
endif

#SvnPath = $(shell find $(LOCAL_PATH) -type d | grep ".svn")
#SourcePath := $(filter-out $(SvnPath), $(SourcePath))

## set the source files.
tmpSourceFiles = $(foreach dir,$(SourcePath),$(wildcard $(dir)/*.c))
SourceFiles = $(foreach file,$(tmpSourceFiles),$(subst $(LOCAL_PATH)/,,$(file)))

## print source files.
# $(warning $(SourceFiles))

## set source file and include path.
LOCAL_SRC_FILES  := $(SourceFiles)
LOCAL_C_INCLUDES := $(SourcePath) \
					$(LOCAL_PATH)/../ \
					$(LOCAL_PATH)/include/

LOCAL_SHARED_LIBRARIES := libcutils libutils

ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL), 1)

LOCAL_CFLAGS +=-DSECUREOS_ENABLED
LOCAL_CFLAGS += $(LAW_CFLAGS)

##communicate proxy on client side.	
LOCAL_SHARED_LIBRARIES+= \
	libtee_client
	
LOCAL_C_INCLUDES += \
    $(TOP)/hardware/aw/client-api
    
endif
## compile libMemAdapter
LOCAL_MODULE:= libMemAdapter

include $(BUILD_SHARED_LIBRARY)
