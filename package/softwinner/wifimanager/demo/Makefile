target			= wifi_connect_ap_test

INCLUDES += -I$(CONFIG_PREFIX)/usr/include

CPP_SRCS = wifi_connect_ap.cpp
CPP_OBJS := $(CPP_SRCS:.cpp=.o)
LIB_OBJS = $(CPP_OBJS)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

#wifi_on_off_test
wifi_on_off_test: wifi_on_off_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_on_off_test $(CONFIG_PREFIX)/usr/bin

#wifi_connect_chinese_ap_test
wifi_connect_chinese_ap_test: wifi_connect_chinese_ap.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_connect_chinese_ap_test $(CONFIG_PREFIX)/usr/bin

#wifi_connect_ap_test
$(CPP_OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(target): $(LIB_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg -lpthread
	@cp -f $(target) $(CONFIG_PREFIX)/usr/bin

#wifi_add_nework_test
wifi_add_network_test: wifi_add_network.cpp
	$(CXX) -o $@ $^ $(CFLAGS) $(INCLUDES) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg -lpthread
	@cp -f wifi_add_network_test $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install $(target) wifi_on_off_test wifi_connect_chinese_ap_test wifi_add_network_test
clean:
	rm -rf $(target) wifi_on_off_test wifi_connect_chinese_ap_test wifi_add_network_test $(LIB_OBJS)

