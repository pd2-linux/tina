TARGET          = cameratest
INCLUDES        += -I. -Icommon/
LIBS            += -lpthread -lm -lrt

PLATFORM_VIDEO = common/sun9iw1p1_video.c
SRCS    = main.c common/hawkview.c common/video_helper.c $(PLATFORM_VIDEO)
OBJS    = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	
all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend

