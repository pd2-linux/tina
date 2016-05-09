
################################################################################
## configurations.
################################################################################

CONFIG_CEDARX_PATH = /home/xuqi/project/cedarx_release/liballwinner

## configure tool chain for linux makefile.
# arm-linux-gnueabihf- or arm-none-linux-gnueabi- tool chain
OPTION_CC_GNUEABIHF = 1
LOCAL_CFLAGS += -DOPTION_CC_GNUEABIHF=$(OPTION_CC_GNUEABIHF)
OPTION_CC_GNUEABI   = 2
LOCAL_CFLAGS += -DOPTION_CC_GNUEABI=$(OPTION_CC_GNUEABI)
OPTION_CC_UCGNUEABI   = 3
LOCAL_CFLAGS += -DOPTION_CC_UCGNUEABI=$(OPTION_CC_UCGNUEABI)
OPTION_CC_LINUX_UCGNUEABI = 4
LOCAL_CFLAGS += -DOPTION_CC_LINUX_UCGNUEABI=$(OPTION_CC_LINUX_UCGNUEABI)
OPTION_CC_LINUX_MUSLGNUEABI = 5
LOCAL_CFLAGS += -DOPTION_CC_LINUX_MUSLGNUEABI=$(OPTION_CC_LINUX_MUSLGNUEABI)
OPTION_CC_LINUX_MUSLGNUEABI64 = 6
LOCAL_CFLAGS += -DOPTION_CC_LINUX_MUSLGNUEABI64=$(OPTION_CC_LINUX_MUSLGNUEABI64)

########## option for os config. ##########
OPTION_OS_ANDROID = 1
LOCAL_CFLAGS += -DOPTION_OS_ANDROID=$(OPTION_OS_ANDROID)
OPTION_OS_LINUX   = 2
LOCAL_CFLAGS += -DOPTION_OS_LINUX=$(OPTION_OS_LINUX)

########## option for os version config. ##########
OPTION_OS_VERSION_ANDROID_4_2 = 1
LOCAL_CFLAGS += -DOPTION_OS_VERSION_ANDROID_4_2=$(OPTION_OS_VERSION_ANDROID_4_2)
OPTION_OS_VERSION_ANDROID_4_4 = 2
LOCAL_CFLAGS += -DOPTION_OS_VERSION_ANDROID_4_4=$(OPTION_OS_VERSION_ANDROID_4_4)
OPTION_OS_VERSION_ANDROID_5_0 = 3
LOCAL_CFLAGS += -DOPTION_OS_VERSION_ANDROID_5_0=$(OPTION_OS_VERSION_ANDROID_5_0)
OPTION_OS_VERSION_ANDROID_6_0 = 4
LOCAL_CFLAGS += -DOPTION_OS_VERSION_ANDROID_6_0=$(OPTION_OS_VERSION_ANDROID_6_0)

########## option for momory driver config. ##########
OPTION_MEMORY_DRIVER_SUNXIMEM = 1
LOCAL_CFLAGS += -DOPTION_MEMORY_DRIVER_SUNXIMEM=$(OPTION_MEMORY_DRIVER_SUNXIMEM)
OPTION_MEMORY_DRIVER_ION      = 2
LOCAL_CFLAGS += -DOPTION_MEMORY_DRIVER_ION=$(OPTION_MEMORY_DRIVER_ION)

########## option for product config. ##########
OPTION_PRODUCT_PAD      = 1
LOCAL_CFLAGS += -DOPTION_PRODUCT_PAD=$(OPTION_PRODUCT_PAD)
OPTION_PRODUCT_TVBOX    = 2
LOCAL_CFLAGS += -DOPTION_PRODUCT_TVBOX=$(OPTION_PRODUCT_TVBOX)
OPTION_PRODUCT_OTT_CMCC = 3
LOCAL_CFLAGS += -DOPTION_PRODUCT_OTT_CMCC=$(OPTION_PRODUCT_OTT_CMCC)
OPTION_PRODUCT_IPTV     = 4
LOCAL_CFLAGS += -DOPTION_PRODUCT_IPTV=$(OPTION_PRODUCT_IPTV)
OPTION_PRODUCT_DVB      = 5
LOCAL_CFLAGS += -DOPTION_PRODUCT_DVB=$(OPTION_PRODUCT_DVB)
OPTION_PRODUCT_LOUDSPEAKER = 6
LOCAL_CFLAGS += -DOPTION_PRODUCT_LOUDSPEAKER=$(OPTION_PRODUCT_LOUDSPEAKER)

########## option for chip config. ##########
OPTION_CHIP_R8 = 1
LOCAL_CFLAGS += -DOPTION_CHIP_R8=$(OPTION_CHIP_R8)
OPTION_CHIP_R16 = 2
LOCAL_CFLAGS += -DOPTION_CHIP_R16=$(OPTION_CHIP_R16)
OPTION_CHIP_C500 = 3
LOCAL_CFLAGS += -DOPTION_CHIP_C500=$(OPTION_CHIP_C500)
OPTION_CHIP_R58 = 4
LOCAL_CFLAGS += -DOPTION_CHIP_R58=$(OPTION_CHIP_R58)
OPTION_CHIP_R18 = 5
LOCAL_CFLAGS += -DOPTION_CHIP_R18=$(OPTION_CHIP_R18)
OPTION_CHIP_T2 = 6
LOCAL_CFLAGS += -DOPTION_CHIP_T2=$(OPTION_CHIP_T2)


########## option for dram interface. ##########
OPTION_DRAM_INTERFACE_DDR1_16BITS = 1
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR1_16BITS=$(OPTION_DRAM_INTERFACE_DDR1_16BITS)
OPTION_DRAM_INTERFACE_DDR1_32BITS = 2
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR1_32BITS=$(OPTION_DRAM_INTERFACE_DDR1_32BITS)
OPTION_DRAM_INTERFACE_DDR2_16BITS = 3
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR2_16BITS=$(OPTION_DRAM_INTERFACE_DDR2_16BITS)
OPTION_DRAM_INTERFACE_DDR2_32BITS = 4
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR2_32BITS=$(OPTION_DRAM_INTERFACE_DDR2_32BITS)
OPTION_DRAM_INTERFACE_DDR3_16BITS = 5
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR3_16BITS=$(OPTION_DRAM_INTERFACE_DDR3_16BITS)
OPTION_DRAM_INTERFACE_DDR3_32BITS = 6
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR3_32BITS=$(OPTION_DRAM_INTERFACE_DDR3_32BITS)
OPTION_DRAM_INTERFACE_DDR3_64BITS = 7
LOCAL_CFLAGS += -DOPTION_DRAM_INTERFACE_DDR3_64BITS=$(OPTION_DRAM_INTERFACE_DDR3_64BITS)

########## option for cmcc ##########
OPTION_CMCC_NO  = 0
LOCAL_CFLAGS += -DOPTION_CMCC_NO=$(OPTION_CMCC_NO)
OPTION_CMCC_YES = 1
LOCAL_CFLAGS += -DOPTION_CMCC_YES=$(OPTION_CMCC_YES)

########## option for dtv ##########
OPTION_DTV_NO  = 0
LOCAL_CFLAGS += -DOPTION_DTV_NO=$(OPTION_DTV_NO)
OPTION_DTV_YES = 1
LOCAL_CFLAGS += -DOPTION_DTV_YES=$(OPTION_DTV_YES)

########## option for AliYUNOS ##########
OPTION_ALI_YUNOS_NO  = 0
LOCAL_CFLAGS += -DOPTION_ALI_YUNOS_NO=$(OPTION_ALI_YUNOS_NO)
OPTION_ALI_YUNOS_YES = 1
LOCAL_CFLAGS += -DOPTION_ALI_YUNOS_YES=$(OPTION_ALI_YUNOS_YES)

########## option for is_camera_decoder ##########
OPTION_IS_CAMERA_DECODER_NO  = 0
LOCAL_CFLAGS += -DOPTION_IS_CAMERA_DECODER_NO=$(OPTION_IS_CAMERA_DECODER_NO)
OPTION_IS_CAMERA_DECODER_YES = 1
LOCAL_CFLAGS += -DOPTION_IS_CAMERA_DECODER_YES=$(OPTION_IS_CAMERA_DECODER_YES)

########## option for ve ipc ##########
OPTION_VE_IPC_DISABLE  = 1
LOCAL_CFLAGS += -DOPTION_VE_IPC_DISABLE=$(OPTION_VE_IPC_DISABLE)
OPTION_VE_IPC_ENABLE = 2
LOCAL_CFLAGS += -DOPTION_VE_IPC_ENABLE=$(OPTION_VE_IPC_ENABLE)

########## option for deinterlace ##########
OPTION_NO_DEINTERLACE = 0
LOCAL_CFLAGS += -DOPTION_NO_DEINTERLACE=$(OPTION_NO_DEINTERLACE)
OPTION_HW_DEINTERLACE = 1
LOCAL_CFLAGS += -DOPTION_HW_DEINTERLACE=$(OPTION_HW_DEINTERLACE)
OPTION_SW_DEINTERLACE = 2
LOCAL_CFLAGS += -DOPTION_SW_DEINTERLACE=$(OPTION_SW_DEINTERLACE)

########## configure linux version ##########
LINUX_VERSION_3_4  = 1
LOCAL_CFLAGS += -DLINUX_VERSION_3_4=$(LINUX_VERSION_3_4)
LINUX_VERSION_3_10 = 2
LOCAL_CFLAGS += -DLINUX_VERSION_3_10=$(LINUX_VERSION_3_10)

########## configure drop delay frame ##########
DROP_DELAY_FRAME_NONE = 0
LOCAL_CFLAGS += -DDROP_DELAY_FRAME_NONE=$(DROP_DELAY_FRAME_NONE)
DROP_DELAY_FRAME_720P = 1
LOCAL_CFLAGS += -DDROP_DELAY_FRAME_720P=$(DROP_DELAY_FRAME_720P)
DROP_DELAY_FRAME_4K = 2
LOCAL_CFLAGS += -DDROP_DELAY_FRAME_4K=$(DROP_DELAY_FRAME_4K)

########## configure 0 copy pixel format ##########
ZEROCOPY_PIXEL_FORMAT_NONE = 0
LOCAL_CFLAGS += -DZEROCOPY_PIXEL_FORMAT_NONE=$(ZEROCOPY_PIXEL_FORMAT_NONE)
ZEROCOPY_PIXEL_FORMAT_YUV_MB32_420 = 1
LOCAL_CFLAGS += -DZEROCOPY_PIXEL_FORMAT_YUV_MB32_420=$(ZEROCOPY_PIXEL_FORMAT_YUV_MB32_420)
ZEROCOPY_PIXEL_FORMAT_YV12 = 2
LOCAL_CFLAGS += -DZEROCOPY_PIXEL_FORMAT_YV12=$(ZEROCOPY_PIXEL_FORMAT_YV12)
ZEROCOPY_PIXEL_FORMAT_NV21 = 3
LOCAL_CFLAGS += -DZEROCOPY_PIXEL_FORMAT_NV21=$(ZEROCOPY_PIXEL_FORMAT_NV21)

########## configure gpu y c align ##########
GPU_Y16_C16_ALIGN = 1
LOCAL_CFLAGS += -DGPU_Y16_C16_ALIGN=$(GPU_Y16_C16_ALIGN)
GPU_Y32_C16_ALIGN = 2
LOCAL_CFLAGS += -DGPU_Y32_C16_ALIGN=$(GPU_Y32_C16_ALIGN)
GPU_Y16_C8_ALIGN = 3
LOCAL_CFLAGS += -DGPU_Y16_C8_ALIGN=$(GPU_Y16_C8_ALIGN)

########## configure gpu y c align ##########
ZEROCOPY_HAL_PIXEL_FORMAT_AW_YUV_PLANNER420 = 1
LOCAL_CFLAGS += -DZEROCOPY_HAL_PIXEL_FORMAT_AW_YUV_PLANNER420=$(ZEROCOPY_HAL_PIXEL_FORMAT_AW_YUV_PLANNER420)
ZEROCOPY_HAL_PIXEL_FORMAT_AW_YVU_PLANNER420 = 2
LOCAL_CFLAGS += -DZEROCOPY_HAL_PIXEL_FORMAT_AW_YVU_PLANNER420=$(ZEROCOPY_HAL_PIXEL_FORMAT_AW_YVU_PLANNER420)

########## configure deinterlace format ##########
DEINTERLACE_FORMAT_MB32_12 = 1
LOCAL_CFLAGS += -DDEINTERLACE_FORMAT_MB32_12=$(DEINTERLACE_FORMAT_MB32_12)
DEINTERLACE_FORMAT_NV = 2
LOCAL_CFLAGS += -DDEINTERLACE_FORMAT_NV=$(DEINTERLACE_FORMAT_NV)
DEINTERLACE_FORMAT_NV21 = 3
LOCAL_CFLAGS += -DDEINTERLACE_FORMAT_NV21=$(DEINTERLACE_FORMAT_NV21)
DEINTERLACE_FORMAT_NV12 = 4
LOCAL_CFLAGS += -DDEINTERLACE_FORMAT_NV12=$(DEINTERLACE_FORMAT_NV12)

########## configure output pixel format ##########
OUTPUT_PIXEL_FORMAT_NV21 = 1
LOCAL_CFLAGS += -DOUTPUT_PIXEL_FORMAT_NV21=$(OUTPUT_PIXEL_FORMAT_NV21)
OUTPUT_PIXEL_FORMAT_YV12 = 2
LOCAL_CFLAGS += -DOUTPUT_PIXEL_FORMAT_YV12=$(OUTPUT_PIXEL_FORMAT_YV12)
OUTPUT_PIXEL_FORMAT_MB32 = 3
LOCAL_CFLAGS += -DOUTPUT_PIXEL_FORMAT_MB32=$(OUTPUT_PIXEL_FORMAT_MB32)

########## configure gpu align stride ##########
GPU_ALIGN_STRIDE_16 = 1
LOCAL_CFLAGS += -DGPU_ALIGN_STRIDE_16=$(GPU_ALIGN_STRIDE_16)
GPU_ALIGN_STRIDE_32 = 2
LOCAL_CFLAGS += -DGPU_ALIGN_STRIDE_32=$(GPU_ALIGN_STRIDE_32)

########## configure ZLIB ##########
OPTION_HAVE_ZLIB = 1
LOCAL_CFLAGS += -DOPTION_HAVE_ZLIB=$(OPTION_HAVE_ZLIB)
OPTION_NO_ZLIB = 2
LOCAL_CFLAGS += -DOPTION_NO_ZLIB=$(OPTION_NO_ZLIB)

########## configure SSL ##########
OPTION_HAVE_SSL = 1
LOCAL_CFLAGS += -DOPTION_HAVE_SSL=$(OPTION_HAVE_SSL)
OPTION_NO_SSL = 2
LOCAL_CFLAGS += -DOPTION_NO_SSL=$(OPTION_NO_SSL)

########## configure LIVE555 ##########
OPTION_HAVE_LIVE555 = 1
LOCAL_CFLAGS += -DOPTION_HAVE_LIVE555=$(OPTION_HAVE_LIVE555)
OPTION_NO_LIVE555 = 2
LOCAL_CFLAGS += -DOPTION_NO_LIVE555=$(OPTION_NO_LIVE555)

#############################################################################
############################## configuration. ############################### 
#############################################################################
CONFIG_CC = $(OPTION_CC_GNUEABI)
CONFIG_OS = $(OPTION_OS_LINUX)
CONFIG_PRODUCT = $(OPTION_PRODUCT_TVBOX)
CONFIG_CHIP = $(OPTION_CHIP_C500)
CONFIG_HAVE_ZLIB = $(OPTION_HAVE_ZLIB)
CONFIG_HAVE_SSL = $(OPTION_HAVE_SSL)
CONFIG_HAVE_LIVE555 = $(OPTION_NO_LIVE555)
#CONFIG_LOG_LEVEL = $(OPTION_LOG_LEVEL_DETAIL)

########## configure CONFIG_CC ##########
LOCAL_CFLAGS += -DCONFIG_CC=$(CONFIG_CC)

########## configure CONFIG_OS ##########
LOCAL_CFLAGS += -DCONFIG_OS=$(CONFIG_OS)

########## configure CONFIG_CHIP##########
LOCAL_CFLAGS += -DCONFIG_CHIP=$(CONFIG_CHIP)

########## configure CONFIG_ALI_YUNOS ##########
CONFIG_ALI_YUNOS = $(OPTION_ALI_YUNOS_NO)
ifdef TARGET_BUSINESS_PLATFORM
    ifeq (aliyunos , $(TARGET_BUSINESS_PLATFORM))
		CONFIG_ALI_YUNOS = $(OPTION_ALI_YUNOS_YES)
    endif
endif
LOCAL_CFLAGS += -DCONFIG_ALI_YUNOS=$(CONFIG_ALI_YUNOS)

########## configure CONFIG_OS_VERSION ########## 
CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_4_4)
ifeq ($(CONFIG_OS), $(OPTION_OS_ANDROID))
    os_version = $(shell echo $(PLATFORM_VERSION) | cut -c 1-3)
    ifeq ($(os_version), 4.2)
        CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_4_2)
    else ifeq ($(os_version), 4.4)
        CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_4_4)
    else ifeq ($(os_version), 5.0)
        CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_5_0)
    else ifeq ($(os_version), 5.1)
        CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_5_0)
    else ifeq ($(os_version), 6.0)
        CONFIG_OS_VERSION = $(OPTION_OS_VERSION_ANDROID_6_0)
    else
        $(warning $(os_version))
        CONFIG_OS_VERSION = -1	
    endif
endif
LOCAL_CFLAGS += -DCONFIG_OS_VERSION=$(CONFIG_OS_VERSION)

########## configure CONFIG_TARGET_PRODUCT ##########
LIB_CEDARM_PATH := $(TOP)/frameworks/av/media/libcedarx
ifeq ($(CONFIG_OS), $(OPTION_OS_ANDROID))
    product = $(shell echo $(TARGET_PRODUCT) | cut -d '_' -f 1)
    ifeq (sugar, $(product)) #A20
        CONFIG_TARGET_PRODUCT=sugar
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/sugar_config.mk
    else ifeq (jaws, $(product)) #A80
        CONFIG_TARGET_PRODUCT=jaws
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/jaws_config.mk
    else ifeq (eagle, $(product)) #H8
        CONFIG_TARGET_PRODUCT=eagle
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/eagle_config.mk
    else ifeq (dolphin, $(product)) #H3
        CONFIG_TARGET_PRODUCT=dolphin
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/dolphin_config.mk
    else ifeq (rabbit, $(product)) #H64
        CONFIG_TARGET_PRODUCT=rabbit
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/rabbit_config.mk
    else ifeq (astar, $(product)) #A33
        CONFIG_TARGET_PRODUCT=astar
        CONFIG_PRODUCT=$(OPTION_PRODUCT_PAD)
        include $(LIB_CEDARM_PATH)/config/astar_config.mk
    else ifeq (octopus, $(product)) #A83
        CONFIG_TARGET_PRODUCT=octopus
        CONFIG_PRODUCT=$(OPTION_PRODUCT_PAD)
        include $(LIB_CEDARM_PATH)/config/octopus_config.mk
    else ifeq (tulip, $(product)) #A64
        CONFIG_TARGET_PRODUCT=tulip
        CONFIG_PRODUCT=$(OPTION_PRODUCT_PAD)
        include $(LIB_CEDARM_PATH)/config/tulip_config.mk
    else ifeq (kylin, $(product)) #A80
        CONFIG_TARGET_PRODUCT=kylin
        CONFIG_PRODUCT=$(OPTION_PRODUCT_PAD)
        include $(LIB_CEDARM_PATH)/config/kylin_config.mk
    else ifeq (magton, $(product)) #V40
        CONFIG_TARGET_PRODUCT=magton
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/magton_config.mk
    else ifeq (aston, $(product)) #V66
        CONFIG_TARGET_PRODUCT=aston
        CONFIG_PRODUCT=$(OPTION_PRODUCT_TVBOX)
        include $(LIB_CEDARM_PATH)/config/aston_config.mk
    else
        $(warning $(product))
    endif
endif
LOCAL_CFLAGS += -DCONFIG_TARGET_PRODUCT=$(CONFIG_TARGET_PRODUCT)

########## configure CONFIG_PRODUCT ##########
LOCAL_CFLAGS += -DCONFIG_PRODUCT=$(CONFIG_PRODUCT)

########## configre zlib ######################
LOCAL_CFLAGS += -DCONFIG_HAVE_ZLIB=$(CONFIG_HAVE_ZLIB)

########## configre ssl ######################
LOCAL_CFLAGS += -DCONFIG_HAVE_SSL=$(CONFIG_HAVE_SSL)

########## configre live555 ######################
LOCAL_CFLAGS += -DCONFIG_HAVE_LIVE555=$(CONFIG_HAVE_LIVE555)

ifeq ($(CONFIG_OS), $(OPTION_OS_LINUX))
	ifeq ($(CONFIG_CHIP),$(OPTION_CHIP_R16))
		include $(CONFIG_CEDARX_PATH)/LIBRARY/config/R16_linux_config.mk   # R16
	else ifeq  ($(CONFIG_CHIP),$(OPTION_CHIP_C500))
		include $(CONFIG_CEDARX_PATH)/LIBRARY/config/C500_linux_config.mk   # c500
	else ifeq  ($(CONFIG_CHIP),$(OPTION_CHIP_R8))
		include $(CONFIG_CEDARX_PATH)/LIBRARY/config/R8_linux_config.mk   # R8
	else
		 $(warning $(CONFIG_CHIP))
	endif
endif
###################################end define####################################
