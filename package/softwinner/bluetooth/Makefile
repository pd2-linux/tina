##############################################
# OpenWrt Makefile for helloworld program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=bluetooth
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

export ARMGCC=$(TARGET_CC)

# This specifies the directory where we're going to build the program. 
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
 
# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/bluetooth/Default
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=Bluetooth APP(bsa 01062)
  DEPENDS:=+libpthread +librt +alsa-lib
endef

define Package/bluetooth
  $(call Package/bluetooth/Default) 
  MENU:=1
  DEFAULT:=1 
endef

define Package/bluetooth-demo
  $(call Package/bluetooth/Default)
  TITLE:=BT app demo
  DEPENDS:=bluetooth +bluetooth
endef

define Package/bluetooth/description
	Bluetooth API
endef

define Package/bluetooth-demo/description
	Bluetooth app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./3rdparty/ $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
  #compile libbtapp
	$(MAKE) -C $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/libbtapp/build \
		-f Makefile \
		ENABLE_ALSA=TRUE \
		CPU=arm \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		STRIP="$(STRIP)" \
		all
	
	#compile bt_demo
	$(MAKE) -C $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test \
		-f Makefile \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(STRIP) $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_demo
	
	#strip bsa_server
	$(STRIP) $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/server/build/arm/bsa_server
	
	#copy bsa_server and apps
	$(CP) -r $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/server/build/arm/bsa_server $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_demo $(PKG_BUILD_DIR)/
	
endef

# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbsa.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtapp.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/bluetooth/install
	$(INSTALL_DIR) $(1)/etc/bluetooth
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bsa_server $(1)/usr/bin/
	$(INSTALL_BIN) ./btenable.sh $(1)/etc/bluetooth/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbsa.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtapp.so $(1)/usr/lib
endef

define Package/bluetooth-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bt_demo $(1)/usr/bin/  
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,bluetooth))
$(eval $(call BuildPackage,bluetooth-demo))
