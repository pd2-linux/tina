TARGET_LIB	:= libaudiomanager.a

INCLUDES        += -I. -I./Apa3165
LIBS            += -lpthread

LIB_SRCS = AudioManager.cpp Apa3165/Apa3165.cpp
OBJS = $(LIB_SRCS:.cpp=.o)

#libipc.so
$(OBJS):%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$(TARGET_LIB):$(OBJS)
	$(AR) r $@ $^
####################################################################
#compile demo
#DEMO := audiomanager_demo

#$(DEMO): main.cpp $(TARGET_LIB)
#	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS)

####################################################################
all: $(TARGET_LIB) $(DEMO) install
install:
	mkdir -p $(CONFIG_PREFIX)/usr/lib
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	mkdir -p $(CONFIG_PREFIX)/usr/include
	cp $(TARGET_LIB) $(CONFIG_PREFIX)/usr/lib
	cp *.h $(CONFIG_PREFIX)/usr/include
	cp ./Apa3165/Apa3165.h $(CONFIG_PREFIX)/usr/include
clean:
	rm -rf $(TARGET_LIB) $(OBJS)

