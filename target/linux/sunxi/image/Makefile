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

RECOVERY_INITRAMFS_BLOCKS=$(shell echo $$(($(CONFIG_SUNXI_RECOVERY_INITRAMFS_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))
OTA_DIR:=$(BIN_DIR)/ota

BOARDS:= \
	R16-Evb-Board \
	R16-Evb-Board-8723BS \
	R16-Bell-One-Board \
	R16-Ol0102-Board \
	R58-Perf-Board \
	R58-Banana-Board

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
	
ifeq ($(CONFIG_TARGET_AW_OTA_INITRAMFS),y)
	rm -f $(BIN_DIR)/boot_initramfs.img
	mkdosfs $(BIN_DIR)/boot_initramfs.img -C $(RECOVERY_INITRAMFS_BLOCKS)
	mcopy -i $(BIN_DIR)/boot_initramfs.img $(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs ::uImage
else
	rm -f $(BIN_DIR)/boot.img
	mkdosfs $(BIN_DIR)/boot.img -C $(FAT32_BLOCKS)
	mcopy -i $(BIN_DIR)/boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs ::uImage
endif

endef
endif

define Image/Build/FirmWare
	$(CP) image.cfg $(BIN_DIR)/bootloader
	./pack_img.sh -r $(BIN_DIR) -c $(2) -d $(3) -b $(4)
endef

define Image/Build/Profile/Default
	$(call Image/Build/FirmWare,$(1),sun8iw5p1, uart0, evb)
endef

define Image/Build/Profile/R16-Evb-Board
	$(call Image/Build/FirmWare,$(1),sun8iw5p1, card0, evb)
endef

define Image/Build/Profile/R16-Ol0102-Board
	$(call Image/Build/FirmWare,$(1),sun8iw5p1, card0, ol0102)
endef

define Image/Build/Profile/R16-Evb-Board-8723BS
	$(call Image/Build/FirmWare,$(1),sun8iw5p1, uart0, evb-8723bs)
endef

define Image/Build/Profile/R16-Bell-One-Board
	$(call Image/Build/FirmWare,$(1),sun8iw5p1, uart0, bell-one)
endef

define Image/Build/Profile/R58-Perf-Board
	$(call Image/Build/FirmWare,$(1),sun8iw6p1, uart0, perf)
endef

define Image/Build/Profile/R58-Banana-Board
	$(call Image/Build/FirmWare,$(1),sun8iw6p1, uart0, banana)
endef

ifeq ($(CONFIG_TARGET_AW_OTA_INITRAMFS),y)
define Image/Build
endef
else
define Image/Build

	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/rootfs.img bs=128k conv=sync
	dd if=$(KDIR)/usr.$(1) of=$(BIN_DIR)/usr.img bs=128k conv=sync

	$(call Image/Build/$(1),$(1))
	$(call Image/Build/Profile/$(PROFILE),$(1))
	
endef

endif

$(eval $(call BuildImage))
