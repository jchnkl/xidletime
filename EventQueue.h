#ifndef __EVENTQUEUE_H
#define __EVENTQUEUE_H

#include <pthread.h>

#include "Deque.h"
#include "HashMap.h"
#include "CommonTypes.h"
#include "PublicConfig.h"
#include "Plugins/PluginConfig.h"

typedef HashMapT WireTableT;

typedef struct EventQueueT
    { unsigned int dynamic
    ; pthread_cond_t wait
    ; pthread_mutex_t lock
    ; DequeT * eventqueue
    ; void (* queueEvent) ( struct EventQueueT *, EventSourceT * )
    ;
    } EventQueueT;

typedef struct EventSourceT
    { IdentT id;
    ; EventQueueT * eq
    ; PublicConfigT * public
    ; PrivateConfig * private
    ; eventRunner er
    ; eventCallback ec
    ;
    } EventSourceT;

typedef struct EventSinkT
    { IdentT id
    ; PublicConfigT * public
    ; PrivateConfig * private
    ; void ( * callback ) ( struct EventSinkT *, EventSourceT * )
    ;
    } EventSinkT;

typedef struct SourceSinkTableT
    { unsigned int   dynamic
    ; unsigned int   numSources;
    ; HashMapT     * eventSources;
    ; unsigned int   numSinks;
    ; HashMapT     * eventSinks;
    ;
    } SourceSinkTableT;

EventQueueT * makeEventQueue ( EventQueueT * );

void destroyEventQueue ( EventQueueT * );

void queueEvent ( EventQueueT *, EventSourceT * );

SourceSinkTableT * makeSourceSinkTable
    ( SourceSinkTableT   *
    , EventQueueT        *
    , PublicConfigT      *
    , EventSourceConfigT *
    , EventSinkConfigT   *
    );

void destroySourceSinkTable ( SourceSinkTableT * );

WireTableT * makeWireTable ( WireTableConfigT *, SourceSinkTableT * );

void destroyWireTable ( WireTableT * );

void startEventSources ( SourceSinkTableT * );

void runEventQueue ( EventQueueT *, WireTableT * );

#endif
