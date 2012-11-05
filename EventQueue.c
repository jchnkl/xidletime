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

void destroySourceSinkTable ( SourceSinkTableT * sst ) {
    destroyHashMap ( sst->eventSources, free );
    destroyHashMap ( sst->eventSinks, free );
    if ( sst->dynamic == 1 ) free ( sst );
}

WireTableT * makeWireTable ( WireTableConfigT * wtc, SourceSinkTableT * sst ) {
    int i = 0, j = 0, k = 0, wtsize = -1;

    while ( wtc[++wtsize].conns != -1 );

    WireTableT * wt = makeHashMap ( NULL, wtsize, NULL );

    for ( i = 0; i < wtsize; ++i ) {
        if ( wtc[i].conns == 0 ) {
            insert ( wt, wtc[i].id, NULL );
        } else {
            DequeT * sinklist  = makeDeque ( NULL );
            for ( j = 0; j < wtc[i].conns; ++j ) {
                for ( k = 0; k < sst->numSinks; ++k ) {
                    // TODO use HashMapT instead of array for sst->sinks
                    if ( sst->sinks[k].id == wtc[i].ids[j] ) {
                        pushHead ( sinklist, &(sst->sinks[k]), NULL );
                        break;
                    }
                }
            }
            insert ( wt, wtc[i].id, sinklist );
        }
    }

    return wt;
}

void startEventSources ( SourceSinkTableT * sst ) {
    int i = 0;
    pthread_t tid;

    for ( i = 0; i < sst->numSources; ++i ) {
        pthread_create ( &tid
                       , NULL
                       , ( void * (*) (void *) )sst->sources[i].er
                       , &(sst->sources[i]) );
    }

}

void runEventQueue
    ( EventQueueT      * eq
    , SourceSinkTableT * sst
    , WireTableT       * wt
    ) {
    while ( 0 == pthread_cond_wait ( &(eq->wait), &(eq->lock) ) ) {
        while ( ! isEmpty ( eq->eventqueue ) ) {
            // EventT * e = popHead ( eq->eventqueue );
            // void * data = eventCallback();
            EventSourceT * es = popHead ( eq->eventqueue );

            DequeT * sinklist = lookup ( wt, es->id );
            if ( sinklist != NULL ) {
                void cb ( EventSinkT * snk ) {
                    snk->callback ( snk, es );
                }
                iterateDequeWith ( sinklist, (void (*) (ElementT *))cb );
            }

        }
    }
}
