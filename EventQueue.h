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
    ;
    } EventQueueT;

EventQueueT * makeEventQueue ( EventQueueT * );

void destroyEventQueue ( EventQueueT * );

void addEvent ( EventQueueT *, EventT * );

void runEventQueue ( EventQueueT * );

#endif
