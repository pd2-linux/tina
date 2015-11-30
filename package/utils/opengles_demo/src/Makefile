TARGET          = opengles_demo
INCLUDES        +=
LIBS           	+=

SRCS    = opengles_demo.c
OBJS    = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	
all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~

