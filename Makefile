CFLAGS=$(shell pkg-config --cflags dbus-1 x11 xext) -Wall
LDFLAGS=$(shell pkg-config --libs dbus-1 x11 xext)

EXE=idletime
OBJ=IdleTime.o XConfig.o DBusSignalEmitter.o IdleMonitor.o

all: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${EXE} $^

%.o: %.c
	${CC} ${CFLAGS} ${LDFLAGS} -c ${CFLAGS} $^

clean:
	rm -rf ${EXE} ${OBJ} *.gch

.PHONY: clean
