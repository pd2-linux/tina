target			= wifitest

INCLUDES += -I$(CONFIG_PREFIX)/usr/include

CPP_SRCS = main.cpp
CPP_OBJS := $(CPP_SRCS:.cpp=.o)
LIB_OBJS = $(CPP_OBJS)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

#wifitest
$(CPP_OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(target): $(LIB_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f $(target) $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install $(target)
clean:
	rm -rf $(target) $(LIB_OBJS)

