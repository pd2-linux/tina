TARGET_RTW_AK := rtw_ak
TARGET_EASYSETUP := easysetup
TARGET_LIBCLENT := client
TARGET_SERVICE := service

HAVE_SOURCE :=  $(wildcard source)

ifeq ($(HAVE_SOURCE),source)
TARGET_LIST := $(TARGET_LIBCLENT) $(TARGET_SERVICE) $(TARGET_EASYSETUP) smartlink_install package_install
else
TARGET_LIST := $(TARGET_LIBCLENT) package_install
endif

#compile client
$(TARGET_LIBCLENT):
	$(MAKE) -C libclient \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

#compile core
$(TARGET_SERVICE):
	$(MAKE) -C source/server \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

#compile broadcom easysetup
$(TARGET_EASYSETUP):	
	$(MAKE) -C source/broadcom/EasySetupTarget/jni \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

#compile realtek rtw_ak
$(TARGET_RTW_AK):
	$(MAKE) -C source/realtek/rtw_ak_r14202 \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

all: $(TARGET_LIST)

smartlink_install:
	cp -r source/server/smartlinkd .
	cp -r source/broadcom/EasySetupTarget/jni/setup .

	cp -r source/server/smartlinkd $(SAVE_DIR)/src/
	cp -r source/broadcom/EasySetupTarget/jni/setup $(SAVE_DIR)/src/

package_install:
	mkdir -p $(CONFIG_PREFIX)/usr/lib
	mkdir -p $(CONFIG_PREFIX)/usr/bin

	cp smartlinkd $(CONFIG_PREFIX)/usr/bin/
	cp setup $(CONFIG_PREFIX)/usr/bin/
	cp -r libclient/libsmartlink_client.so $(CONFIG_PREFIX)/usr/lib  