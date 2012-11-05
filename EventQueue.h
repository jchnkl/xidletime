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

EventQueueT * makeEventQueue ( EventQueueT * );

void destroyEventQueue ( EventQueueT * );

void queueEvent ( EventQueueT *, EventSourceT * );

void runEventQueue ( EventQueueT * );

#endif
