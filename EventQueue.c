#include "EventQueue.h"

#include <stdlib.h>

EventQueueT * makeEventQueue ( EventQueueT * eq ) {
    if ( eq == NULL ) {
        eq = (EventQueueT *) calloc ( 1, sizeof ( EventQueueT ) );
        eq->dynamic = 1;
    } else {
        eq->dynamic = 0;
    }

    return eq;
}

void destroyEventQueue ( EventQueueT * eq ) {
    destroyDeque ( eq->eventqueue, NULL );
    if ( eq->dynamic ) free ( eq );
}

void addEvent ( EventQueueT * eq, EventT * e ) {
    pushLast ( eq->eventqueue, e, NULL );
}

void runEventQueue ( EventQueueT * eq ) {
    while ( ! isEmpty ( eq->eventqueue ) ) {
        EventT * e = popHead ( eq->eventqueue );
        e->callback ( e->data );
    }
}
