#ifndef __DBUSINTERFACE_H
#define __DBUSINTERFACE_H

#include <dbus/dbus.h>

#include "../EventQueue.h"

typedef struct DBusConfig
    { const char     * busName
    ; const char     * objectPath
    ; const char     * interfaceName
    ; const char     * signalName
    ; DBusConnection * connection
    ;
    } DBusConfig;

void dbusSendSignalSink ( EventSinkT *, EventSourceT * );

#endif
