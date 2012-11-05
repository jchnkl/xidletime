#include "EventQueue.h"

#include <stdlib.h>

EventQueueT * makeEventQueue ( EventQueueT * eq ) {
    if ( eq == NULL ) {
        eq = (EventQueueT *) calloc ( 1, sizeof ( EventQueueT ) );
        eq->dynamic = 1;
    } else {
        eq->dynamic = 0;
    }

    eq->eventqueue = makeDeque ( NULL );

    pthread_cond_init ( &(eq->wait), NULL );
    pthread_mutex_init ( &(eq->lock), NULL );
    pthread_mutex_lock ( &(eq->lock) );

    eq->queueEvent = queueEvent;

    return eq;
}

void destroyEventQueue ( EventQueueT * eq ) {
    destroyDeque ( eq->eventqueue, NULL );
    pthread_cond_destroy ( &(eq->wait) );
    pthread_mutex_destroy ( &(eq->lock) );
    if ( eq->dynamic ) free ( eq );
}

void addEvent ( EventQueueT * eq, EventT * e ) {
    pushLast ( eq->eventqueue, e, NULL );
    pthread_cond_signal ( &eq->wait );
}

void runEventQueue ( EventQueueT * eq ) {
    while ( 0 == pthread_cond_wait ( &(eq->wait), &(eq->lock) ) ) {
        while ( ! isEmpty ( eq->eventqueue ) ) {
            EventT * e = popHead ( eq->eventqueue );
            e->callback ( e->data );
        }
    }
}
