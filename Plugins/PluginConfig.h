#ifndef __PLUGINCONFIG_H
#define __PLUGINCONFIG_H

#include <stddef.h>

#include "../CommonTypes.h"
#include "../EventQueue.h"

typedef struct EventSourceConfigT
    { IdentT id;
    ; PrivateConfig * private
    ; eventRunner er
    ; eventCallback ec
    ;
    } EventSourceConfigT;

typedef struct EventSinkConfigT
    { IdentT id
    ; PrivateConfig * private
    ; void ( * callback ) ( struct EventSinkT *, EventSourceT * )
    ;
    } EventSinkConfigT;


typedef struct WireTableConfigT
    { size_t conns
    ; IdentT id
    ; IdentT ids[128]
    ;
    } WireTableConfigT;

#endif
