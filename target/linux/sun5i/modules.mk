#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
define KernelPackage/net-rtl8188eu
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8188EU support
  DEPENDS:=@TARGET_sun5i
  KCONFIG:=CONFIG_RTL8188EU
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8188eu/8188eu.ko
  AUTOLOAD:=$(call AutoProbe,8188eu)
endef

define KernelPackage/net-rtl8188eu/description
 Kernel modules for RealTek RTL8188EU support
endef

$(eval $(call KernelPackage,net-rtl8188eu))

define KernelPackage/net-rtl8723bs
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8723BS support
  DEPENDS:=@TARGET_sun5i
  KCONFIG:=CONFIG_RTL8723BS
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8723bs/8723bs.ko
  AUTOLOAD:=$(call AutoProbe,8723bs)
endef

define KernelPackage/net-rtl8723bs/description
 Kernel modules for RealTek RTL8723BS support
endef

$(eval $(call KernelPackage,net-rtl8723bs))

define KernelPackage/net-ap6212
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=ap6212 support
  DEPENDS:=@TARGET_sun5i 
  FILES:=$(LINUX_DIR)/drivers/net/wireless/bcmdhd/bcmdhd.ko
  AUTOLOAD:=$(call AutoProbe,bcmdhd)
endef

define KernelPackage/net-ap6212/description
 Kernel modules for Broadcom AP6212  support
endef

$(eval $(call KernelPackage,net-ap6212))

define KernelPackage/sun5i-csi
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=sun5i-csi support
  DEPENDS:=@TARGET_sun5i
  FILES:=$(LINUX_DIR)/drivers/media/video/videobuf-core.ko \
  	 $(LINUX_DIR)/drivers/media/video/videobuf-dma-contig.ko \
  	 $(LINUX_DIR)/drivers/media/video/sun5i_csi/device/gc0308.ko \
  	 $(LINUX_DIR)/drivers/media/video/sun5i_csi/csi0/sun5i_csi0.ko
  AUTOLOAD:=$(call AutoLoad,90,videobuf-core videobuf-dma-contig gc0308 sun5i_csi0)
endef

define KernelPackage/sun5i-csi/description
  Kernel modules for sun5i-csi support
endef

$(eval $(call KernelPackage,sun5i-csi))

define KernelPackage/net-esp8089
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=esp8089 support
  DEPENDS:=@TARGET_sun5i +esp8089-firmware
  FILES:=$(LINUX_DIR)/drivers/net/wireless/esp8089/esp8089.ko
endef

define KernelPackage/net-esp8089/description
 Kernel modules for net-esp8089  support
endef

$(eval $(call KernelPackage,net-esp8089))
