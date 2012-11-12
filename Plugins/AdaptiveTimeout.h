#ifndef __ADAPTIVETIMEOUT_H
#define __ADAPTIVETIMEOUT_H

#include <sys/time.h>

#include "../EventQueue.h"
#include "../GetOptions.h"

#include "KMeansCluster.h"

typedef struct AdaptiveTimeoutT
    { GroupsT        * groups
    ; const char     * idlefile
    ; const char     * timeoutfile
    ; double           base
    ; unsigned int     idletime
    ; struct timeval   lastTime
    ; int              class[2]
    ;
    } AdaptiveTimeoutT;

void adaptiveTimeoutSink ( EventSinkT *, EventSourceT * );

#endif
