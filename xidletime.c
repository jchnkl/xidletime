#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "IdleTimer.h"
#include "group.h"
#include "GetOptions.h"
#include "kmeans.h"
#include "SignalHandler.h"
#include "DBusSignalEmitter.h"

typedef unsigned  int uint;
typedef unsigned long ulong;

GroupData globalGroupData;

typedef struct CallbackData
    { Options       * options
    ; SignalEmitter * signalemitter
    ; GroupData     * groupdata
    ; int             newIdletime
    ; int             class[2]
    ;
    } CallbackData;

static void idleTimerCallback
    ( TimerStatusT
    , IdleTimerData *
    , XSyncAlarmNotifyEvent *
    , void *
    );

static void signalHandler ( int, siginfo_t *, void * );

int main ( int argc, char ** argv ) {

    CallbackData cd; memset ( &cd, 0, sizeof ( CallbackData ) );

    Options options;
    getoptions ( &options, argc, argv );
    cd.options = &options;
    cd.newIdletime = options.idletime * 1000;

    const char * seed[2];
    seed[0] = options.idlefile;
    seed[1] = options.timeoutfile;

    group_t group[2];
    int size[] = { 100, 10 };
    cmp_type_t comp[] = { MEAN, FILL };
    int initMeans ( int m, int s ) {
        return (int)((double)(options.idletime * 1000) * (double)m / (double)s);
    }

    memset ( &globalGroupData, 0, sizeof ( GroupData ) );
    globalGroupData.init = initMeans;
    globalGroupData.ngroups = 2;
    globalGroupData.group = group;
    globalGroupData.size = size;
    globalGroupData.comp = comp;
    globalGroupData.seed = seed;
    initGroups ( &globalGroupData );

    cd.class[0]  = size[0] - 1;
    cd.class[1]  = size[1] - 1;
    cd.groupdata = &globalGroupData;

    int signals[] = { SIGINT, SIGTERM, SIGUSR1 };
    installSignalHandler ( 3, signals, signalHandler );

    ulong flags = XSyncCACounter
                | XSyncCAValueType
                | XSyncCATestType
                | XSyncCAValue
                | XSyncCADelta
                ;

    XSyncAlarmAttributes attributes;

    IdleTimerData itd; memset ( &itd, 0, sizeof ( IdleTimerData ) );
    itd.flags = flags;
    itd.attributes = &attributes;
    itd.idletime = options.idletime * 1000;

    initIdleTimer ( &itd );

    // initialize dbus signal emitter
    DBusConfig dbusconfig;
    dbusconfig.busName       = options.busName;
    dbusconfig.objectPath    = options.objectPath;
    dbusconfig.interfaceName = options.interfaceName;

    initDBus ( &dbusconfig );
    SignalEmitter signalemitter;
    getSignalEmitter ( &dbusconfig, &signalemitter );

    cd.signalemitter = &signalemitter;
    // dbus signal emitter init done

    runTimer ( &itd, (void *)&cd, idleTimerCallback );

    dumpGroup ( &group[0], seed[0] );
    finalizeGroup ( &group[0] );

    dumpGroup ( &group[1], seed[1] );
    finalizeGroup ( &group[1] );

    finalizeDBus ( &dbusconfig );

    return 0;

}

static void idleTimerCallback
    ( TimerStatusT timerstatus
    , IdleTimerData * itd
    , XSyncAlarmNotifyEvent * alarmEvent
    , void * data
    ) {

    CallbackData  *   cd =  (CallbackData *) data;
    Options       * opts =       (Options *) cd->options;
    SignalEmitter *  se  = (SignalEmitter *) cd->signalemitter;
    GroupData     *   gd =     (GroupData *) cd->groupdata;

    if ( timerstatus == Reset ) {
        fprintf ( stderr, "Reset\n" );

        uint time = alarmEvent->time - itd->lastEventTime;

        cd->class[0] = addValue ( &(gd->group[0]), &time );

        FILE * stream = fopen ( gd->seed[0], "a" );
        fwrite ( &time, sizeof ( uint ), 1, stream );
        fclose ( stream );

        // gnuplot:
        // base=0.2
        // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)

        // double base = strtod ( argv[1], NULL );
        double base = opts->base;
        double prob = -1.0 * log(gd->size[0]/base) / log(base)
                    + log(cd->class[0] + 1.0) / log(base);

        double weight = (cd->class[1] + 1.0) / (double)(gd->size[1]);

        int newtime = (double)(cd->newIdletime) * weight * prob;

        if ( newtime >= itd->idletime ) {
            cd->newIdletime = newtime;

            cd->class[1] = addValue ( &(gd->group[1]), (uint *) &newtime );

            FILE * stream = fopen ( gd->seed[1], "a" );
            fwrite ( &newtime, sizeof ( uint ), 1, stream );
            fclose ( stream );

            XSyncValue value;
            XSyncIntToValue ( &value, newtime );
            itd->attributes->trigger.wait_value = value;
        }

#ifdef DEBUG_CALLBACK
    fprintf ( stderr
            , "time: %u\tclass[0]: %i\tclass[1]: %i\nprob: %f\tweight: %f\tnewtime: %i\n"
            , time
            , cd->class[0]
            , cd->class[1]
            , prob
            , weight
            , newtime
            );
#endif

    } else {
        fprintf ( stderr, "Idle\n" );
    }

}

static void signalHandler ( int sig, siginfo_t * siginfo, void * context ) {
    int g;

#ifdef DEBUG_SIGNALHANDLER
    fprintf ( stderr, "Caught signal %d\n", sig );
#endif

    switch (sig) {
        case SIGUSR1:
            for ( g = 0; g < globalGroupData.ngroups; g++ ) {
                printGroup ( &(globalGroupData.group[g]) );
            }
            break;

        default:
            for ( g = 0; g < globalGroupData.ngroups; g++ ) {
                dumpGroup ( &(globalGroupData.group[g])
                          ,   globalGroupData.seed[g]
                          );
            }
            exit ( EXIT_SUCCESS );
    }
}
