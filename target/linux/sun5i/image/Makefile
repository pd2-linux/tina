#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/host.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_SUNXI_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

BOARDS:= \
	R8M-Evb-Board \
	R8-Evb-Board \
	R8-NextThing-Board \
	R8-ChangHong-Board \
	R8-Slbh-Board \
	R8-Dev-Board

ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
define Image/BuildKernel
	mkimage -A arm -O linux -T kernel -C none \
	-a 0x40008000 -e 0x40008000 \
	-n 'AW Tina Linux-$(LINUX_VERSION)' \
	-d $(KDIR)/zImage $(BIN_DIR)/$(IMG_PREFIX)-uImage

	rm -f $(BIN_DIR)/boot.img
	mkdosfs $(BIN_DIR)/boot.img -C $(FAT32_BLOCKS)
	
	mcopy -i $(BIN_DIR)/boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage ::uImage

endef
else
define Image/BuildKernel
	$(CP) $(KDIR)/zImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs
	echo -ne '\x00\x00\x00\x00' >> $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs
	$(call Image/BuildKernel/MkuImage, \
		none, 0x40008000, 0x40008000, \
		$(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs, \
		$(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs \
		)
	rm -f $(BIN_DIR)/boot.img
	mkdosfs $(BIN_DIR)/boot.img -C $(FAT32_BLOCKS)
	
	mcopy -i $(BIN_DIR)/boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs ::uImage
endef
endif

define Image/Build/FirmWare
	$(CP) image.cfg $(BIN_DIR)/bootloader
	./pack_img.sh -r $(BIN_DIR) -c $(2) -d $(3) -b $(4)
endef

define Image/Build/Profile/Default
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, evb)
endef

define Image/Build/Profile/R8M-Evb-Board
	$(call Image/Build/FirmWare,$(1),sun5iw1p2, uart0, evb)
endef

define Image/Build/Profile/R8-Evb-Board
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, evb)
endef

define Image/Build/Profile/R8-NextThing-Board
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, nextthing)
endef

define Image/Build/Profile/R8-ChangHong-Board
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, changhong)
endef

define Image/Build/Profile/R8-Slbh-Board
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, slbh)
endef

define Image/Build/Profile/R8-Dev-Board
	$(call Image/Build/FirmWare,$(1),sun5i, uart0, dev)
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/rootfs.img bs=128k conv=sync
	$(call Image/Build/$(1),$(1))
	$(call Image/Build/Profile/$(PROFILE),$(1))
	
endef

$(eval $(call BuildImage))
