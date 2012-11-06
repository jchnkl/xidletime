CFLAGS=-g -Wall -lm -pthread
CFLAGS+=$(shell pkg-config --cflags dbus-1 x11 xscrnsaver)
LDFLAGS=$(shell pkg-config --libs   dbus-1 x11 xscrnsaver)

EXE=eventtest

OBJ=eventtest.o \
    EventQueue.o \
    Deque.o \
    HashMap.o \
    GetOptions.o \
    PublicConfig.o

OBJSRC=eventtest.c \
       EventQueue.c \
       Deque.c \
       HashMap.c \
       GetOptions.c \
       PublicConfig.c

PLUGINOBJS=PluginConfig.o \
           KMeansCluster.o \
           HelloWorld.o \
           XIdleTimer.o \
           AdaptiveTimeout.o \
           DBusInterface.o

PLUGINSRCS=Plugins/PluginConfig.c \
           Plugins/KMeansCluster.c \
           Plugins/HelloWorld.c \
           Plugins/XIdleTimer.c \
           Plugins/AdaptiveTimeout.c \
           Plugins/DBusInterface.c

define cc-command
${CC} ${CFLAGS} ${LDFLAGS} -c ${CFLAGS} $^
endef

all:
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJSRC} ${PLUGINSRCS} -o ${EXE} $^


%.o: %.c
	${cc-command}

clean:
	rm -rf ${EXE} ${OBJ} ${PLUGINOBJS} *.gch

.PHONY: clean
