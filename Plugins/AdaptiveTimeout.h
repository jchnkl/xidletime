#ifndef __ADAPTIVETIMEOUT_H
#define __ADAPTIVETIMEOUT_H

#include <sys/time.h>

#include "../EventQueue.h"
#include "../GetOptions.h"

#include "KMeansCluster.h"

typedef struct TimerCallbackT
    { GroupsT        * groups
    ; Options        * options
    ; struct timeval   lastTime
    ; int              class[2]
    ;
    } TimerCallbackT;

void adaptiveTimeoutSink ( EventSinkT *, EventSourceT * );

#endif
