#ifndef __DBUSSIGNALEMITTER_H
#define __DBUSSIGNALEMITTER_H

#include <dbus/dbus.h>

#include "SignalEmitter.h"

typedef struct DBusConfig
    { const char     * busName
    ; const char     * objectPath
    ; const char     * interfaceName
    ; const char     * signalName
    ; DBusConnection * connection
    ;
    } DBusConfig;

int initDBus ( DBusConfig * dbusconfig );

void finalizeDBus ( DBusConfig * dbusconfig );

int dbusEmitSignal ( DBusConfig * dbusconfig );

int getSignalEmitter ( DBusConfig * dbusconfig, SignalEmitter * signalemitter );

#endif
