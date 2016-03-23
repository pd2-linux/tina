LOCAL_PATH:= $(call my-dir)

################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

DEMUX_PATH = $(LOCAL_PATH)/../..
LIBRARY_PATH = $(DEMUX_PATH)/..

LOCAL_SRC_FILES:= testMp4Muxer.c \
									

LOCAL_C_INCLUDES := \
  $(LOCAL_PATH) \
  $(DEMUX_PATH)/BASE/include \
  $(DEMUX_PATH)/STREAM/include \
  $(DEMUX_PATH)/PARSER/include \
$(DEMUX_PATH)/MUXER/include \
  $(DEMUX_PATH)/PARSER/mov \
	$(LIBRARY_PATH) \
	$(LIBRARY_PATH)/CODEC/VIDEO/ENCODER/include/ \
$(LIBRARY_PATH)/CODEC/AUDIO/ENCODER/include/ \
$(LIBRARY_PATH)/CODEC/VIDEO/DECODER/include/ \
$(LIBRARY_PATH)/CODEC/AUDIO/DECODER/include/ \
$(LIBRARY_PATH)/CODEC/SUBTITLE/DECODER/include/ \


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libVE \
	libMemAdapter \
	libvencoder \
	libcdx_muxer \
	libcdx_stream \
	libcdx_parser \
	libcdx_base \
        libcdx_muxer

LOCAL_MODULE:= demoMp4Muxer

include $(BUILD_EXECUTABLE)

