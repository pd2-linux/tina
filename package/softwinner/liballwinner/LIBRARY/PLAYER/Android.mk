LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(LOCAL_PATH)/../config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    audioDecComponent.cpp                 \
    audioRenderComponent.cpp              \
    videoDecComponent.cpp                 \
    subtitleDecComponent.cpp              \
    subtitleRenderComponent.cpp           \
    soundControl/soundControl_android.cpp \
    soundControl/soundControl_android_raw.cpp \
    soundControl/IEC61937.c               \
    avtimer.cpp                           \
    messageQueue.cpp                      \
    bitrateEstimater.cpp                  \
    framerateEstimater.cpp                \
    streamManager.cpp                     \
    player.cpp                            \
    deinterlace.cpp                       \
    deinterlaceHw.cpp                     \
    layerControl/layerControl_android.cpp
    
    
ifeq ($(USE_SW_DEINTERLACE), yes)
LOCAL_SRC_FILES += \
    sw-deinterlace/deinterlaceSw.cpp 	\
    sw-deinterlace/postprocess.cpp      \
    sw-deinterlace/AWPostProcess.cpp    \
    sw-deinterlace/yadif.cpp

endif

ifeq ($(USE_NEW_DISPLAY),1)
LOCAL_SRC_FILES += \
    layerControl/layerControl_android_newDisplay.cpp \
    videoRenderComponent_newDisplay.cpp
else
LOCAL_SRC_FILES += \
    videoRenderComponent.cpp              
endif

ifeq ($(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX),1)
LOCAL_SRC_FILES += \
    subtitleNativeDisplay/subtitleNativeDisplay.cpp
endif

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                    \
        $(TOP)/frameworks/av/include/            \
        $(TOP)/frameworks/native/include/android/            \
        $(LOCAL_PATH)/../CODEC/VIDEO/DECODER/include    \
        $(LOCAL_PATH)/../CODEC/AUDIO/DECODER/include    \
        $(LOCAL_PATH)/../CODEC/SUBTITLE/DECODER/include \
        $(LOCAL_PATH)/../VE/include                     \
        $(LOCAL_PATH)/../MEMORY/include                 \
        $(LOCAL_PATH)/../                        \
        $(LOCAL_PATH)/include                        \
        $(LOCAL_PATH)/subtitleNativeDisplay             \
        external/skia/include/core \
		external/skia/include/effects \
		external/skia/include/images \
		external/skia/src/ports \
		external/skia/src/core \
		external/skia/include/utils \
		external/icu4c/common 

ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_5_0))
ifeq ($(CONFIG_CHIP), $(OPTION_CHIP_1667))
LOCAL_C_INCLUDES  += $(TOP)/hardware/aw/hwc/astar/
endif
endif

ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_6_0))
ifeq ($(CONFIG_CHIP), $(OPTION_CHIP_1667))
LOCAL_C_INCLUDES  += $(TOP)/hardware/aw/hwc/astar/
endif
endif

LOCAL_MODULE_TAGS := optional

TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS += $(LAW_CFLAGS) 

LOCAL_CFLAGS += -Wno-deprecated-declarations
#LOCAL_CFLAGS += -Werror -Wno-deprecated-declarations

LOCAL_MODULE:= libplayer

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
        libmedia            \
        libui               \
        libgui              \
        libvdecoder	    \
        libadecoder         \
        libsdecoder         \
        libMemAdapter       \
        libtinyalsa        \
        libion \
        libskia \
        libicuuc

include $(BUILD_SHARED_LIBRARY)

