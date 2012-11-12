#ifndef __PUBLICCONFIG_H
#define __PUBLICCONFIG_H

#include <pthread.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <dbus/dbus.h>

#include "GetOptions.h"

typedef struct PublicConfigT
    { pthread_mutex_t   publiclock // don't touch this; use withPublicConfig;
    ; unsigned int      dynamic    // makePublicConfig & destroyPublicConfig
    ; Options         * options
    ; xcb_connection_t * c
    ; DBusConnection  * dbusconn
    ;
    } PublicConfigT;

PublicConfigT * makePublicConfig ( PublicConfigT * );

void destroyPublicConfig ( PublicConfigT * );

void withPublicConfig ( PublicConfigT *, void ( * ) ( PublicConfigT * ) );

#endif
