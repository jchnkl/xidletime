#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "Callback.h"
#include "XTimer.h"
#include "GetOptions.h"
#include "KMeansCluster.h"
#include "SignalHandler.h"
#include "DBusSignalEmitter.h"

typedef struct CallbackData
    { Options       *  options
    ; SignalEmitter *  signalemitter
    ; GroupsT       *  groups
    ; int              newIdletime
    ; int              class[2]
    ;
    } CallbackData;

static void idleTimerCallback ( CallbackDataT * callback );

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

    GroupT group[2];
    unsigned int size[] = { 100, 10 };
    CmpTypeT comp[] = { MEAN, FILL };
    int initMeans ( int m, int s ) {
        return (int)((double)(options.idletime * 1000) * (double)m / (double)s);
    }

    GroupsT groups;
    groups.ngroups = 2;
    groups.groups  = &group[0];
    makeGroups ( initMeans, &groups, size, comp, seed );
    cd.groups    = &groups;

    cd.class[0]  = size[0] - 1;
    cd.class[1]  = size[1] - 1;

    int signals[] = { SIGINT, SIGTERM, SIGUSR1 };
    initializeSignalData ( &groups );
    installSignalHandler ( 3, signals, signalHandler );

    ulong flags = XSyncCACounter
                | XSyncCAValueType
                | XSyncCATestType
                | XSyncCAValue
                | XSyncCADelta
                ;

    XSyncAlarmAttributes attributes;

    XTimerT itd; memset ( &itd, 0, sizeof ( XTimerT ) );
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

    CallbackT callback;
    IdleTimerCallbackT idletimercallback;
    idletimercallback.data = &cd;
    callback.data = &idletimercallback;
    callback.run  = idleTimerCallback;

    runTimer ( &itd, &callback );

    dumpGroup ( &group[0] );
    finalizeGroup ( &group[0] );

    dumpGroup ( &group[1] );
    finalizeGroup ( &group[1] );

    finalizeDBus ( &dbusconfig );

    return 0;

}

static void idleTimerCallback ( CallbackDataT * data ) {

    IdleTimerCallbackT    * itc        = (IdleTimerCallbackT *) data;
    XTimerT         * itd        = itc->itd;
    XSyncAlarmNotifyEvent * alarmEvent = itc->xsane;
    CallbackData          * cd         = (CallbackData       *) itc->data;

    Options               * opts       = (Options            *) cd->options;
    SignalEmitter         * se         = (SignalEmitter      *) cd->signalemitter;
    GroupsT               * groups     = (GroupsT            *) cd->groups;

    if ( itc->status == Reset ) {
#ifdef DEBUG_CALLBACK
        fprintf ( stderr, "Reset\n" );
#endif
        se->emitSignal ( se, "Reset" );

        uint time = alarmEvent->time - itd->lastEventTime;

        cd->class[0] = addValue ( &(groups->groups[0]), &time );

        FILE * stream = fopen ( groups->groups[0].seed, "a" );
        fwrite ( &time, sizeof ( uint ), 1, stream );
        fclose ( stream );

        // gnuplot:
        // base=0.2
        // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)

        // double base = strtod ( argv[1], NULL );
        double base = opts->base;
        double prob = -1.0 * log(groups->groups[0].size/base) / log(base)
                    + log(cd->class[0] + 1.0) / log(base);

        double weight = (cd->class[1] + 1.0) / (double)(groups->groups[1].size);

        int newtime = (double)(cd->newIdletime) * weight * prob;

        if ( newtime >= itd->idletime ) {
            cd->newIdletime = newtime;

            cd->class[1] = addValue ( &(groups->groups[1]), (uint *) &newtime );

            FILE * stream = fopen ( groups->groups[1].seed, "a" );
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
#ifdef DEBUG_CALLBACK
        fprintf ( stderr, "Idle\n" );
#endif
        se->emitSignal ( se, "Idle" );
    }

}

static void signalHandler ( int sig, siginfo_t * siginfo, void * context ) {
    int g;
    GroupsT * groups = (GroupsT *) getSignalData();

#ifdef DEBUG_SIGNALHANDLER
    fprintf ( stderr, "Caught signal %d\n", sig );
#endif

    switch (sig) {
        case SIGUSR1:
            for ( g = 0; g < groups->ngroups; g++ ) {
                printGroup ( &(groups->groups[g]) );
            }
            break;

        default:
            for ( g = 0; g < groups->ngroups; g++ ) {
                dumpGroup ( &(groups->groups[g]) );
            }
            exit ( EXIT_SUCCESS );
    }
}
