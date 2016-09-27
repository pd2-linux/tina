TARGET          = cpu_use_test
INCLUDES        += -I. -Icommon/
LIBS            += -lpthread -lm -lrt

SRCS    = cpu_use_test.c
OBJS    = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC)  -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	
all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend

