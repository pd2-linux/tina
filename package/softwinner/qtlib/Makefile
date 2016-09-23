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
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=qtlib
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

HAVE_SRC := no

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
define Package/qtlib
  SECTION:=utils
  CATEGORY:=softwinner
  TITLE:=qtlib for openwrt
  DEPENDS:= +libstdc++ +librt
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/qtlib/description
	it comes form Android 4.2!!
endef
 
# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/qtlib/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/bin/qtapp
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./lib $(1)/qtlib
	cp -fpR ./qtlib $(1)/lib/
	$(INSTALL_DATA) ./etc/ts.conf $(1)/etc/ts.conf
	$(INSTALL_DATA) ./etc/pointercal $(1)/etc/pointercal
	$(INSTALL_BIN) ./etc/startqt $(1)/etc/init.d/startqt
	$(INSTALL_BIN) ./bin/BranQt4 $(1)/bin/qtapp/BranQt4
	$(INSTALL_BIN) ./bin/BranTest $(1)/bin/qtapp/BranTest
	$(INSTALL_BIN) ./bin/baklight $(1)/bin/qtapp/baklight
	$(INSTALL_BIN) ./bin/bran.config $(1)/bin/qtapp/bran.config
	$(INSTALL_BIN) ./bin/BranTip.config $(1)/bin/qtapp/BranTip.config
	$(INSTALL_BIN) ./bin/startqt.sh $(1)/bin/qtapp/startqt.sh
	$(INSTALL_BIN) ./bin/esp_fcc.conf $(1)/bin/esp_fcc.conf


endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,qtlib))
