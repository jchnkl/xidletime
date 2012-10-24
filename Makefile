CFLAGS=$(shell pkg-config --cflags dbus-1 x11 xext) -Wall -lm
LDFLAGS=$(shell pkg-config --libs dbus-1 x11 xext)

EXE=xidletime
OBJ=xidletime.o \
    XTimer.o \
    GetOptions.o \
    KMeansCluster.o \
    SignalHandler.o \
    DBusSignalEmitter.o

all: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${EXE} $^

%.o: %.c
	${CC} ${CFLAGS} ${LDFLAGS} -c ${CFLAGS} $^

clean:
	rm -rf ${EXE} ${OBJ} *.gch

.PHONY: clean
