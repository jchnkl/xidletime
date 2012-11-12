#ifndef __DBUSINTERFACE_H
#define __DBUSINTERFACE_H

#include <dbus/dbus.h>

#include "../EventQueue.h"

typedef struct DBusT
    { const char     * busName
    ; const char     * objectPath
    ; const char     * interfaceName
    ; const char     * signalName
    ; DBusConnection * connection
    ;
    } DBusT;

void * dbusReceiveSource ( EventSourceT * );

void dbusEmitSignalSink ( EventSinkT *, EventSourceT * );

#endif
