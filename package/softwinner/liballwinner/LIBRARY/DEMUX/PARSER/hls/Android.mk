LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk
include $(LIB_ROOT)/PARSER/config.mk

LOCAL_SRC_FILES = \
                $(notdir $(wildcard $(LOCAL_PATH)/*.c))

ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_6_0))
	LOCAL_C_INCLUDES:= \
		$(LIB_ROOT)/BASE/include \
	    $(LIB_ROOT)/STREAM/include \
	    $(LIB_ROOT)/PARSER/include \
		$(TOP)/external/boringssl/src/include \
	    $(LIB_ROOT)/../CODEC/VIDEO/DECODER/include    \
	    $(LIB_ROOT)/../CODEC/AUDIO/DECODER/include    \
	    $(LIB_ROOT)/../CODEC/SUBTITLE/DECODER/include \
	    $(LIB_ROOT)/../PLAYER/                 \
	    $(LIB_ROOT)/../     
else
	LOCAL_C_INCLUDES:= \
		$(LIB_ROOT)/BASE/include \
	    $(LIB_ROOT)/STREAM/include \
	    $(LIB_ROOT)/PARSER/include \
		$(TOP)/external/openssl/include \
	    $(LIB_ROOT)/../CODEC/VIDEO/DECODER/include    \
	    $(LIB_ROOT)/../CODEC/AUDIO/DECODER/include    \
	    $(LIB_ROOT)/../CODEC/SUBTITLE/DECODER/include \
	    $(LIB_ROOT)/../PLAYER/                 \
	    $(LIB_ROOT)/../     
endif

		
LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_hls_parser

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)

