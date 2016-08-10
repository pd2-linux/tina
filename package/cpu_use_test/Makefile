# 这些 makefile 字文件确立软件包加入 OpenWrt 的方式和方法
include $(TOPDIR)/rules.mk

# PKG_NAME 		: 包名
# PKG_VERSION	: 软件包版本号
# PKG_RELEASE	: Makefile 的版本号
PKG_NAME:=cpu_use_test
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


# 软件包编译目录 
# 它的父目录为$(BUILD_DIR)。如果不指定
# 默认为$(BUILD_DIR)/$( PKG_NAME)/$( PKG_VERSION)
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

# 一般在软包的基本信息完成后再引入
include $(INCLUDE_DIR)/package.mk
 
# 应用程序编译包以 "Package/" 开头，然后接着软件名(可以与软件包名不一样)
# SECTION 	: 包的类型
# CATEGORY	: 包的分类，在 make menuconfig 的菜单显示的名字
# TITLE		: 用于在 make menuconfig 对该软件包的简述
# DEPENDS	: 表示依赖其他软件。即如编译或安装需要其他软件时需要说明。如果
#			  存在多个依赖，则每个依赖需要用空格分开。依赖前使用+号表示默认
#			  为显示，即对象沒有选中时也会显示，使用@则默认为不显示，即当依
#			  赖对象选中后才显示。
define Package/cpu_use_test
  SECTION:=utils
  CATEGORY:=ethan_apps
  TITLE:=is cpu_use_test bin file
  DEPENDS:=+libpthread +libjson-c
endef

# 软件包的详细描述
define Package/cpu_use_test/description
	is 11111111111111111111111111
endef

# 编译准备方法
#	对于网上下载的软件包不需要再描述
#	对于非网上下载或自行开发的软件包必须说明编译准备方法
# 以下为一般的编译准备方法，按 OpenWrt 的习惯，一般把自己设计的程序全部放在 src 目录下
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# 编译方法,如果不定义将使用默认的编译方法 Build/Compile/Default
# 自行开发的软件考虑使用下面的定义
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="-lpthread -ljson-c" \
		all
endef

# 软件包的安装方法
#	包括一系列拷贝编译好的文件到指定位置。
#	调用时会带一个参数，就是嵌入系统的镜像文件系统目录
#	因此 $(1) 表示嵌入系统的镜像目录。一般可以采用下面的方法
# INSTALL_DIR\INSTALL_BIN	: 在 $(TOPDIR)/rules.mk 定义
# INSTALL_DIR	: INSTALL_DIR :=install -d -m0755 意思是创建所属用户可读写
#				  其他用户可读可执行的目录
# INSTALL_BIN	: INSTALL_BIN :=install -m0755 意思是编译好的文件存放到镜像文件目录

define Package/cpu_use_test/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpu_use_test $(1)/bin/
endef

# PDF 文档描述，在 boot 阶段启动
#define Package/cpu_use_test/install
#	$(INCLUDE_DIR) $(1)/sbin/ $(1)/etc/config/ $(1)/etc/init.d/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpu_use_test $(1)/sbin/
#	$(INCLUDE_DIR) ./files/cpu_use_test.config $(1)/etc/config/cpu_use_test
#	$(INSTALL_BIN) ./files/cpu_use_test.init $(1)/etc/init.d/cpu_use_test
#endef


# 完成前面定义后，必须使用 eval 函数实现各种定义(感觉就像是这句才是真正执行上面的动作)
$(eval $(call BuildPackage,cpu_use_test))
