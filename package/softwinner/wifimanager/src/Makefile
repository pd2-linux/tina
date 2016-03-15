target			= libwifimg.so

INCLUDES        += -I. -I./include
LIBS            += -lpthread -ldl

SRCS =wifimanager.c wifi_event.c wifi.c udhcpc_thread.c

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/wifi_intf.h $(CONFIG_PREFIX)/usr/include

#libwifimg.so
$(target): $(SRCS) libwpa_cli.a
	$(CXX) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

