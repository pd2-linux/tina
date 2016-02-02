TARGET = libsmartlink_client.so
LOCAL_FLAGS =  -I../include \
			   -I../include/platform/Tina \

$(TARGET) : connect.c
	$(CC) $(FLAGS) $(LDFLAGS) $(LOCAL_FLAGS) -o $@ $^ -shared -fPIC
