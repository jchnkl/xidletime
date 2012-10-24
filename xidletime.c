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

typedef struct TimerCallbackT
    { Options       *  options
    ; SignalEmitter *  signalemitter
    ; GroupsT       *  groups
    ; int              newIdletime
    ; int              class[2]
    ; CallbackDataT * data;
    ;
    } TimerCallbackT;

// static void timerCallback ( CallbackT * callback );
static void timerCB ( CallbackDataT * );

static void signalHandler ( int, siginfo_t *, void * );

int main ( int argc, char ** argv ) {

    TimerCallbackT timercb; memset ( &timercb, 0, sizeof ( TimerCallbackT ) );

    Options options;
    getoptions ( &options, argc, argv );
    timercb.options = &options;
    timercb.newIdletime = options.idletime;

    const char * seed[2];
    seed[0] = options.idlefile;
    seed[1] = options.timeoutfile;

    GroupT group[2];
    unsigned int size[] = { 100, 10 };
    CmpTypeT comp[] = { MEAN, FILL };
    int initMeans ( int m, int s ) {
        return (int)((double)(options.idletime) * (double)m / (double)s);
    }

    GroupsT groups;
    groups.ngroups = 2;
    groups.groups  = &group[0];
    makeGroups ( initMeans, &groups, size, comp, seed );
    timercb.groups    = &groups;

    timercb.class[0]  = size[0] - 1;
    timercb.class[1]  = size[1] - 1;

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

    XTimerT xtimer; memset ( &xtimer, 0, sizeof ( XTimerT ) );
    xtimer.flags = flags;
    xtimer.attributes = &attributes;
    xtimer.idletime = options.idletime;

    initXTimer ( &xtimer );

    // initialize dbus signal emitter
    DBusConfig dbusconfig;
    dbusconfig.busName       = options.busName;
    dbusconfig.objectPath    = options.objectPath;
    dbusconfig.interfaceName = options.interfaceName;

    initDBus ( &dbusconfig );
    SignalEmitter signalemitter;
    getSignalEmitter ( &dbusconfig, &signalemitter );

    timercb.signalemitter = &signalemitter;
    // dbus signal emitter init done

    CallbackT callback;
    IdleTimerCallbackT idletimercallback;
    idletimercallback.data = &timercb;
    callback.data = &idletimercallback;
    callback.run  = timerCB;

    runXTimer ( &xtimer, &callback );

    dumpGroup ( &group[0] );
    finalizeGroup ( &group[0] );

    dumpGroup ( &group[1] );
    finalizeGroup ( &group[1] );

    finalizeDBus ( &dbusconfig );

    return 0;

}

static void timerCB ( CallbackDataT * data ) {

    IdleTimerCallbackT    * itc        = (IdleTimerCallbackT *) data;
    XTimerT               * xtimer     = itc->xtimer;
    XSyncAlarmNotifyEvent * alarmEvent = itc->xsane;

    TimerCallbackT        * timercb    = (TimerCallbackT     *) itc->data;
    Options               * opts       = (Options            *) timercb->options;
    SignalEmitter         * se         = (SignalEmitter      *) timercb->signalemitter;
    GroupsT               * groups     = (GroupsT            *) timercb->groups;

    if ( itc->status == Reset ) {
#ifdef DEBUG_CALLBACK
        fprintf ( stderr, "Reset\n" );
#endif
        se->emitSignal ( se, "Reset" );

        uint time = alarmEvent->time - xtimer->lastEventTime;

        timercb->class[0] = addValue ( &(groups->groups[0]), &time );

        FILE * stream = fopen ( groups->groups[0].seed, "a" );
        fwrite ( &time, sizeof ( uint ), 1, stream );
        fclose ( stream );

        // gnuplot:
        // base=0.2
        // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)

        // double base = strtod ( argv[1], NULL );
        double base = opts->base;
        double prob = -1.0 * log(groups->groups[0].size/base) / log(base)
                    + log(timercb->class[0] + 1.0) / log(base);

        double weight = (timercb->class[1] + 1.0) / (double)(groups->groups[1].size);

        int newtime = (double)(timercb->newIdletime) * weight * prob;

        if ( newtime >= xtimer->idletime ) {
            timercb->newIdletime = newtime;

            timercb->class[1] = addValue ( &(groups->groups[1]), (uint *) &newtime );

            FILE * stream = fopen ( groups->groups[1].seed, "a" );
            fwrite ( &newtime, sizeof ( uint ), 1, stream );
            fclose ( stream );

            setXIdleTime ( xtimer, newtime );
            // XSyncValue value;
            // XSyncIntToValue ( &value, newtime );
            // xtimer->attributes->trigger.wait_value = value;
        }

#ifdef DEBUG_CALLBACK
    fprintf ( stderr
            , "time: %u\tclass[0]: %i\tclass[1]: %i\nprob: %f\tweight: %f\tnewtime: %i\n"
            , time
            , timercb->class[0]
            , timercb->class[1]
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
