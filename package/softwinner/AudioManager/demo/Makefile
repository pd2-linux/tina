####################################################################
#compile demo

INCLUDE := -I../src/ -I../src/Apa3165
DEMO := audiomanager_demo
TARGET_LIB := libaudiomanager.a

$(DEMO): main.cpp $(TARGET_LIB)
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS) $(INCLUDE) -lpthread

####################################################################
all: $(DEMO) install
install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin

	cp $(DEMO) $(CONFIG_PREFIX)/usr/bin
clean:
	rm -rf $(DEMO) $(OBJS)

