DIR_CUR := $(shell pwd)

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

BT ?= 0
VR ?= 0
LAPSULE ?= 0
DMR ?= 0
DMS ?= 0
AIRPLAY ?= 0
LOCALPLAYER ?= 0

#AR = $(AR)
#CC = $(CC)
#CXX = $(CXX)

INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin

MOLIB_DIR=../../output/molib/app
MOLIB_UPDATER_DIR=../../output/molib/updater
MOZART_DIR=../../output/mozart/app

# common opts.
#CFLAGS = -fPIC -Wall -g -O2 -I. -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include
LOCAL_CFLAGS = -fPIC -Wall -I. -I./include -g -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include
#LDFLAGS = -g -lmad -lstdc++ -L. -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LOCAL_LDFLAGS = -g -lasound -lmad -L. -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
#LDFLAGS += -lutils -lini

SRCS += ${wildcard *.c ./libaudio/*.c}
OBJS = $(patsubst %.c, %.c.o, $(SRCS))
TARGET = libtinymp3.a 

all:$(TARGET) tinyplayer install

tinyplayer:$(TARGET)
	$(CC) -o tinyplayer tinyplayer.c -g -Wall -ltinymp3 -I. -L. $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS)

$(TARGET):$(OBJS)
	$(AR) r $@ $(OBJS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
	#$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@

clean:
	-rm -rf $(OBJS) $(TARGET) tinyplayer

install:
	-mkdir -p $(CONFIG_PREFIX)/bin 
	-cp tinyplayer $(CONFIG_PREFIX)/bin/

.PHONY:all clean $(TARGET) install
