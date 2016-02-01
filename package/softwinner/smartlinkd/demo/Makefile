CC ?= g++

TARGET          = client
INCLUDES        += -I. \
                   -I../client_base \
		   -I../include \
		   -I../include/platform/Tina \

SRCS := \
	main.c \
	../client_base/connect.c

SUB_LIB := \
	-lpthread


$(TARGET): $(SRCS) $(SUB_LIB)
	$(CC) $(LDFLAGS) $(INCLUDES) $(SRCS) $(SUB_LIB) -o $@

#all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
