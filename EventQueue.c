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

SourceSinkTableT * makeSourceSinkTable
    ( SourceSinkTableT   * sst
    , EventQueueT        * eq
    , PublicConfig       * public
    , EventSourceConfigT * srccfg
    , EventSinkConfigT   * snkcfg
    ) {

    int i = 0, numsrcs = -1, numsnks = -1;

    if ( sst == NULL ) {
        sst = (SourceSinkTableT *) calloc ( 1, sizeof ( SourceSinkTableT ) );
    }

    while ( srccfg[++numsrcs].id != -1 );
    while ( snkcfg[++numsnks].id != -1 );

    sst->numSources = numsrcs;
    sst->numSinks   = numsnks;

    sst->sources = (EventSourceT *) calloc ( numsrcs, sizeof ( EventSourceT ) );
    sst->sinks = (EventSinkT *) calloc ( numsnks, sizeof ( EventSinkT ) );

    for ( i = 0; i < numsrcs; ++i ) {
        sst->sources[i].id = srccfg[i].id;
        sst->sources[i].eq = eq;
        sst->sources[i].public = public;
        sst->sources[i].private = srccfg[i].private;
        sst->sources[i].er = srccfg[i].er;
        sst->sources[i].ec = srccfg[i].ec;
    }

    for ( i = 0; i < numsrcs; ++i ) {
        sst->sinks[i].id = snkcfg[i].id;
        sst->sinks[i].public = public;
        sst->sinks[i].private = snkcfg[i].private;
        sst->sinks[i].callback = snkcfg[i].callback;
    }

    return sst;
}

void runEventQueue ( EventQueueT * eq ) {
    while ( 0 == pthread_cond_wait ( &(eq->wait), &(eq->lock) ) ) {
        while ( ! isEmpty ( eq->eventqueue ) ) {
            EventT * e = popHead ( eq->eventqueue );
            e->callback ( e->data );
        }
    }
}