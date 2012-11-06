#ifndef __ADAPTIVETIMEOUT_H
#define __ADAPTIVETIMEOUT_H

#include <sys/time.h>

#include "../EventQueue.h"
#include "../GetOptions.h"

#include "KMeansCluster.h"

typedef struct ATConfigT
    { GroupsT        * groups
    ; const char     * idlefile
    ; const char     * timeoutfile
    ; double           base
    ; unsigned int     idletime
    ; struct timeval   lastTime
    ; int              class[2]
    ;
    } ATConfigT;

void adaptiveTimeoutSink ( EventSinkT *, EventSourceT * );

#endif
