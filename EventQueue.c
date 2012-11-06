#include "EventQueue.h"

#include <stdlib.h>

EventQueueT * makeEventQueue ( EventQueueT * eq ) {
    if ( eq == NULL ) {
        eq = (EventQueueT *) calloc ( 1, sizeof ( EventQueueT ) );
        eq->dynamic = 1;
    } else {
        eq->dynamic = 0;
    }

    pthread_cond_init ( &(eq->wait), NULL );
    pthread_mutex_init ( &(eq->lock), NULL );
    pthread_mutex_lock ( &(eq->lock) );

    eq->eventqueue = makeDeque ( NULL );
    eq->queueEvent = queueEvent;

    return eq;
}

void destroyEventQueue ( EventQueueT * eq ) {
    destroyDeque ( eq->eventqueue, NULL );
    pthread_cond_destroy ( &(eq->wait) );
    pthread_mutex_destroy ( &(eq->lock) );
    if ( eq->dynamic ) free ( eq );
}

void queueEvent ( EventQueueT * eq, EventSourceT * es ) {
    pushLast ( eq->eventqueue, es, NULL );
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
        sst->dynamic = 1;
    } else {
        sst->dynamic = 0;
    }

    while ( srccfg[++numsrcs].id != -1 );
    while ( snkcfg[++numsnks].id != -1 );

    sst->numSources = numsrcs;
    sst->numSinks   = numsnks;

    sst->eventSources = makeHashMap ( NULL, numsrcs, NULL );
    sst->eventSinks   = makeHashMap ( NULL, numsnks, NULL );

    for ( i = 0; i < numsrcs; ++i ) {
        EventSourceT * es = calloc ( 1, sizeof ( EventSourceT ) );
        es->id      = srccfg[i].id;
        es->eq      = eq;
        es->public  = public;
        es->private = srccfg[i].private;
        es->er      = srccfg[i].er;
        es->ec      = srccfg[i].ec;
        insert ( sst->eventSources, es->id, es );
    }

    for ( i = 0; i < numsnks; ++i ) {
        EventSinkT * es = calloc ( 1, sizeof ( EventSinkT ) );
        es->id       = snkcfg[i].id;
        es->public   = public;
        es->private  = snkcfg[i].private;
        es->callback = snkcfg[i].callback;
        insert ( sst->eventSinks, es->id, es );
    }

    return sst;
}

void destroySourceSinkTable ( SourceSinkTableT * sst ) {
    destroyHashMap ( sst->eventSources, free );
    destroyHashMap ( sst->eventSinks, free );
    if ( sst->dynamic == 1 ) free ( sst );
}

WireTableT * makeWireTable ( WireTableConfigT * wtc, SourceSinkTableT * sst ) {
    int i = 0, j = 0, wtsize = -1;

    while ( wtc[++wtsize].conns != -1 );

    WireTableT * wt = makeHashMap ( NULL, wtsize, NULL );

    for ( i = 0; i < wtsize; ++i ) {
        if ( wtc[i].conns == 0 ) {
            insert ( wt, wtc[i].id, NULL );
        } else {
            DequeT * sinklist = makeDeque ( NULL );
            for ( j = 0; j < wtc[i].conns; ++j ) {
                pushHead ( sinklist
                         , lookup ( sst->eventSinks, wtc[i].ids[j] )
                         , NULL
                         );
            }
            insert ( wt, wtc[i].id, sinklist );
        }
    }

    return wt;
}

void startEventSources ( SourceSinkTableT * sst ) {
    pthread_t tid;

    void start_thread ( EventSourceT * es ) {
        pthread_create ( &tid, NULL, (void * (*) (void *))es->er, es );
    }

    iterateHashMapWith ( sst->eventSources, (void (*) (void *))start_thread );
}

void runEventQueue
    ( EventQueueT * eq
    , WireTableT  * wt
    ) {
    while ( 0 == pthread_cond_wait ( &(eq->wait), &(eq->lock) ) ) {
        while ( ! isEmpty ( eq->eventqueue ) ) {
            EventSourceT * es = popHead ( eq->eventqueue );

            DequeT * sinklist = lookup ( wt, es->id );
            if ( sinklist != NULL ) {
                void cb ( EventSinkT * snk ) { snk->callback ( snk, es ); }
                iterateDequeWith ( sinklist, (void (*) (ElementT *))cb );
            }

        }
    }
}
