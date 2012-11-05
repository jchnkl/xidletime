#ifndef __EVENTQUEUE_H
#define __EVENTQUEUE_H

#include <pthread.h>

#include "Deque.h"

typedef struct EventT
    { void * data
    ; void ( * callback ) ( void * )
    ;
    } EventT;

typedef struct EventQueueT
    { pthread_cond_t wait
    ; pthread_mutex_t lock
    ; DequeT * eventqueue
    ; unsigned int dynamic
    ; void (* queueEvent) ( struct EventQueueT *, EventSourceT * )
    ;
    } EventQueueT;

typedef struct EventSourceT
    { IdentT id;
    ; EventQueueT * eq
    ; PublicConfig * public
    ; PrivateConfig * private
    ; eventRunner er
    ; eventCallback ec
    ;
    } EventSourceT;

typedef struct EventSinkT
    { IdentT id
    ; PublicConfig * public
    ; PrivateConfig * private
    ; void ( * callback ) ( struct EventSinkT *, EventSourceT * )
    ;
    } EventSinkT;

typedef struct SourceSinkTableT
    { unsigned int   numSources;
    ; EventSourceT * sources;
    ; unsigned int   numSinks;
    ; EventSinkT   * sinks;
    ;
    } SourceSinkTableT;

EventQueueT * makeEventQueue ( EventQueueT * );

void destroyEventQueue ( EventQueueT * );

void queueEvent ( EventQueueT *, EventSourceT * );

void runEventQueue ( EventQueueT * );

SourceSinkTableT * makeSourceSinkTable
    ( SourceSinkTableT   *
    , EventQueueT        *
    , PublicConfig       *
    , EventSourceConfigT *
    , EventSinkConfigT   *
    );


#endif
