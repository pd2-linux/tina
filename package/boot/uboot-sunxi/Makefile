#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=u-boot
PKG_VERSION:=2011.09
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
  PROFILE:=
endef

define uboot/R16-Evb-Board
  TITLE:=U-Boot 2011.09 Pre-Built for R16
  CONFIG:=sun8iw5p1
  PROFILE:=evb
endef  

define uboot/R16-Ol0102-Board
  TITLE:=U-Boot 2011.09 Pre-Built for R16
  CONFIG:=sun8iw5p1
  PROFILE:=ol0102
endef

define uboot/R16-Bell-One-Board
  TITLE:=U-Boot 2011.09 Pre-Built for R16
  CONFIG:=sun8iw5p1
  PROFILE:=bell-one
endef  

define uboot/R16-Evb-Board-8723BS
  TITLE:=U-Boot 2011.09 Pre-Built for R16
  CONFIG:=sun8iw5p1
  PROFILE:=evb-8723bs
endef  

define uboot/R58-Perf-Board
  TITLE:=U-Boot 2011.09 Pre-Built for R58
  CONFIG:=sun8iw6p1
  PROFILE:=perf
endef  

define uboot/R58-Banana-Board
  TITLE:=U-Boot 2011.09 Pre-Built for R58
  CONFIG:=sun8iw6p1
  PROFILE:=banana
endef  

define uboot/A80-Evb-Board-Binary
  TITLE:=U-Boot 2011.09 Pre-Built for A80
  CONFIG:=sun9iw1p1
  PROFILE:=evb
endef

UBOOTS:=R16-Evb-Board \
	R16-Ol0102-Board \
	R16-Bell-One-Board \
	R16-Evb-Board-8723BS \
	R58-Perf-Board \
	R58-Banana-Board \
	A80-Evb-Board-Binary

define Package/uboot/template
define Package/uboot-sunxi-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_sunxi
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  MAINTAINER:=Trace Wong<wangyaliang@allwinnertech.com>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),openwrt-$(BOARD)-$(BUILD_VARIANT)-u-boot.bin)
endif

define Build/Configure
endef

define Build/Compile
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/bootloader
	$(CP) env.cfg $(BIN_DIR)/bootloader/
	$(CP) bootloader/$(CONFIG)/bin/boot0_nand_$(CONFIG).bin $(BIN_DIR)/bootloader/boot0_nand.fex
	$(CP) bootloader/$(CONFIG)/bin/boot0_sdcard_$(CONFIG).bin $(BIN_DIR)/bootloader/boot0_sdcard.fex
	$(CP) bootloader/$(CONFIG)/bin/boot0_spinor_$(CONFIG).bin $(BIN_DIR)/bootloader/boot0_spinor.fex
	$(CP) bootloader/$(CONFIG)/bin/fes1_$(CONFIG).bin $(BIN_DIR)/bootloader/fes1.fex
	$(CP) bootloader/$(CONFIG)/bin/u-boot-$(CONFIG).bin $(BIN_DIR)/bootloader/u-boot.fex
	$(CP) bootloader/$(CONFIG)/bin/u-boot-spinor-$(CONFIG).bin $(BIN_DIR)/bootloader/u-boot-spinor.fex
#	$(CP) bootloader/$(CONFIG)/bin/semelis_$(CONFIG).bin $(BIN_DIR)/bootloader/semelis.bin
#	$(CP) bootloader/$(CONFIG)/bin/sboot_$(CONFIG).bin $(BIN_DIR)/bootloader/sboot.bin

	$(CP) bootloader/$(CONFIG)/configs/$(PROFILE)/* $(BIN_DIR)/bootloader/
	$(CP) bootloader/tools/* $(BIN_DIR)/bootloader/
	$(CP) bootloader/boot-resource/boot-resource $(BIN_DIR)/bootloader/
	$(CP) bootloader/boot-resource/boot-resource.ini $(BIN_DIR)/bootloader/
endef

define Package/uboot/install/template
define Package/uboot-sunxi-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-sunxi-$(u))) \
)
