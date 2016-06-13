LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../../..
include $(LIB_ROOT)/config.mk

DEMUX_PATH = $(LIB_ROOT)/DEMUX

include $(DEMUX_PATH)/STREAM/config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES:= \
    $(LIB_ROOT)/EXTERNAL/include/live \
	$(TOP)/external/openssl/include \
	$(DEMUX_PATH)/BASE/include \
	$(DEMUX_PATH)/STREAM/include \
    $(DEMUX_PATH)/PARSER/include \
	$(LIB_ROOT)/CODEC/AUDIO/DECODER/include \
	$(LIB_ROOT)/CODEC/VIDEO/DECODER/include \
    $(LIB_ROOT)/CODEC/SUBTITLE/DECODER/include \
	$(LIB_ROOT)


LOCAL_CFLAGS += $(CDX_CFLAGS) -Wno-unused

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_rtsp_stream

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)

