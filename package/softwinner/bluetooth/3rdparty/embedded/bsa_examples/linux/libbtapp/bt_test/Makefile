#bt_demo

INCLUDES += -I$(CONFIG_PREFIX)/usr/include

target := bt_demo

src_cpp := bt_demo.cpp
objs_cpp := $(src_cpp:.cpp=.o)
objs := $(objs_cpp)

.PHONY: all clean
all:	$(target)

$(objs_cpp): %.o : %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(objs)
	@echo CONFIG_PREFIX
	@echo '$(CONFIG_PREFIX)'
	@echo Linking application $@ with libbtapp.so.
	$(CXX) $(LDFLAGS) -o $@ $^ -L$(CONFIG_PREFIX)/usr/lib -lbtapp -lbsa

clean:
	@echo Cleaning up : $(objs) $(target)
	@rm -rf $(objs) $(target)
	@echo Finished cleaning up
