#include "AdaptiveTimeout.h"

#ifdef DEBUG_CALLBACK
#include <sys/ioctl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "XIdleTimer.h"

static TimerCallbackT staticTimerCB;

static TimerCallbackT * initAdaptiveTimeoutSink
    ( EventSourceT   * src
    , TimerCallbackT * timercb
    ) {
    timercb = &staticTimerCB;
    memset ( timercb, 0, sizeof ( TimerCallbackT ) );

    Options * options = calloc ( 1, sizeof ( Options ) );
    // getoptions ( &options, argc, argv );
    options->base = 0.15; options->idletime = 7000;
    options->idlefile = "/tmp/idlefile.dat";
    options->timeoutfile = "/tmp/timeoutfile.dat";

    timercb->options = options;

    const char * seed[2];
    seed[0] = options->idlefile;
    seed[1] = options->timeoutfile;

    // GroupT group[2];
    GroupT * group = calloc ( 2, sizeof ( GroupT ) );
    unsigned int size[] = { 100, 10 };
    CmpTypeT comp[] = { MEAN, FILL };
    int initMeans ( int m, int s ) {
        return (int)((double)(options->idletime) * (double)m / (double)s);
    }

    // GroupsT groups;
    GroupsT * groups = calloc ( 1, sizeof ( GroupsT ) );
    groups->ngroups = 2;
    groups->groups  = group;
    makeGroups ( initMeans, groups, size, comp, seed );
    timercb->groups = groups;

    timercb->class[0]  = size[0] - 1;
    timercb->class[1]  = size[1] - 1;
    gettimeofday ( &timercb->lastTime, NULL );

    return timercb;
}

void adaptiveTimeoutSink ( EventSinkT * snk, EventSourceT * src ) {
    TimerCallbackT * timercb = (TimerCallbackT *) snk->private;
    if ( timercb == NULL ) {
        timercb = initAdaptiveTimeoutSink ( src, timercb );
        snk->private = (void *)timercb;
    }

    Options       * options    = (Options       *) timercb->options;
    GroupsT       * groups     = (GroupsT       *) timercb->groups;

    XTimerCallbackT * xtcallback = (XTimerCallbackT *)src->private;
    XTimerT         * xtimer     = xtcallback->xtimer;

    FILE * stream;
    struct timeval tv;
    uint time, newtime;
    double prob, weight, base = options->base;

    gettimeofday ( &tv, NULL );

    if ( xtcallback->status == Reset ) {

        time = tv.tv_sec * 1000 + tv.tv_usec / 1000
             - timercb->lastTime.tv_sec * 1000
             - timercb->lastTime.tv_usec / 1000;

        timercb->class[0] = addValue ( &(groups->groups[0]), &time );

        stream = fopen ( groups->groups[0].seed, "a" );
        fwrite ( &time, sizeof ( uint ), 1, stream );
        fclose ( stream );

        // gnuplot:
        // base=0.2
        // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)

        prob = -1.0 * log(groups->groups[0].size/base) / log(base)
             + log(timercb->class[0] + 1.0) / log(base);

        weight = (timercb->class[1] + 1.0) / (double)(groups->groups[1].size);

        newtime = (double)( getXIdleTime ( xtimer ) ) * weight * prob;

        if ( newtime >= options->idletime ) {
            timercb->class[1] = addValue ( &(groups->groups[1]), (uint *) &newtime );

            stream = fopen ( groups->groups[1].seed, "a" );
            fwrite ( &newtime, sizeof ( uint ), 1, stream );
            fclose ( stream );

            setXIdleTime ( xtimer, newtime );
        }

#ifdef DEBUG_CALLBACK
    struct winsize ws;
    ioctl ( 0, TIOCGWINSZ, &ws );

    typedef enum TypT { INT=0, UINT, ULONG, DOUBLE } TypT;

    void showTypedValue ( char * dst, TypT typ, void * val ) {
        switch (typ) {
            case INT:    sprintf ( dst, "%i",    * (int *) val ); break;
            case UINT:   sprintf ( dst, "%u",   * (uint *) val ); break;
            case ULONG:  sprintf ( dst, "%lu", * (ulong *) val ); break;
            case DOUBLE: sprintf ( dst, "%f", * (double *) val ); break;
        }
    }

    typedef struct DebugInfoT
        { char * txt
        ; TypT typ
        ; void * val
        ;
        } DebugInfoT;

    DebugInfoT debuginfo[] =
        { { "time: ",     UINT,   &time                }
        , { "class[0]: ", INT,    &(timercb->class[0]) }
        , { "class[1]: ", INT,    &(timercb->class[1]) }
        , { "prob: ",     DOUBLE, &prob                }
        , { "weight: ",   DOUBLE, &weight              }
        , { "newtime: ",  UINT,   &newtime             }
        , { NULL,         0,      NULL                 }
        };

    uint n = -1, printed = 0, maxlen = 16; char buf[maxlen];
    while ( debuginfo[++n].txt != NULL ) {
        showTypedValue ( buf, debuginfo[n].typ, debuginfo[n].val );
        printed += fprintf ( stderr, "%s%s\t", debuginfo[n].txt, buf );
        if ( printed + maxlen > ws.ws_col ) {
            printed = 0;
            fprintf ( stderr, "\n" );
        }
    }
    fprintf ( stderr, "\n" );

#endif

    }

    timercb->lastTime = tv;

}
