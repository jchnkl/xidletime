#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "kMeans.h"

typedef struct signalData
    { int ngroups
    ; group_t * groups
    ; const char ** seed
    ;
    } signalData;

signalData globalSignalData;

long XSyncValueToLong ( XSyncValue *value );

static void installSignalHandler ( int nsignals, int * signals );

static void signalHandler ( int sig, siginfo_t * siginfo, void * context );

static
void initGroups ( int (* init) (int)
                , int ngroups
                , group_t * group
                , int * size
                , const char ** seed
                );

int main ( int argc, char ** argv ) {

    int i = 0, k = 0
      , major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , numCounter = 0
      , myIdleTime = strtol ( argv[2], NULL, 10 ) * 1000
      , nwIdleTime = myIdleTime
      ;

    const char * seed[2];
    seed[0] = argv[3]; // idleFile
    seed[1] = argv[4]; // timeoutFile

    group_t groups[2];
    int size[] = { 100, 10 };
    int initMeans ( int v ) { return myIdleTime * v; }
    initGroups ( initMeans, 2, groups, size, seed );

    memset ( &globalSignalData, 0, sizeof ( signalData ) );
    globalSignalData.ngroups = 2;
    globalSignalData.groups = groups;
    globalSignalData.seed = seed;

    int signals[] = { SIGINT, SIGTERM, SIGUSR1 };
    installSignalHandler ( 3, signals );

    XEvent xEvent;
    Time lastEventTime = 0;
    XSyncValue value[3];
    XSyncAlarmNotifyEvent * alarmEvent = NULL;

    XSyncAlarm alarm[2];

    XSyncSystemCounter * sysCounter = NULL, * counter = NULL;

    Display *dpy = XOpenDisplay ("");
    Window root = DefaultRootWindow ( dpy );

    XSelectInput ( dpy, root, XSyncAlarmNotifyMask );

    XSyncInitialize ( dpy, &major, &minor );

    XSyncQueryExtension ( dpy , &ev_base , &err_base );

    sysCounter = XSyncListSystemCounters ( dpy, &numCounter );

    for (i = 0; i < numCounter; i++) {
        if ( 0 == strcmp ( sysCounter[i].name, "IDLETIME" ) ) {
            counter = &sysCounter[i];
        }
    }

    XSyncIntToValue ( &value[0], 0 );
    XSyncIntToValue ( &value[1], myIdleTime );

    XSyncAlarmAttributes attributes[1];

    attributes[0].trigger.counter    = counter->counter;
    attributes[0].trigger.value_type = XSyncAbsolute;
    attributes[0].trigger.test_type  = XSyncPositiveComparison;
    attributes[0].trigger.wait_value = value[1];
    attributes[0].delta              = value[0];

    unsigned long flags = XSyncCACounter
                        | XSyncCAValueType
                        | XSyncCATestType
                        | XSyncCAValue
                        | XSyncCADelta
                        ;

    alarm[0] = XSyncCreateAlarm ( dpy, flags, &attributes[0] );

    int class[2];
    class[0] = size[0] - 1;
    class[1] = size[1] - 1;

    while ( 1 ) {
        XNextEvent ( dpy, &xEvent );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( alarmEvent->alarm == alarm[0] ) {
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                attributes[0].trigger.test_type = XSyncPositiveComparison;

                if ( lastEventTime != 0 && lastEventTime < alarmEvent->time ) {

                    unsigned int newtime = 0;
                    unsigned int time = alarmEvent->time - lastEventTime;

                    class[0] = addValue ( &groups[0], &time );

                    FILE * stream = fopen ( seed[0], "a" );
                    fwrite ( &time, sizeof ( unsigned int ), 1, stream );
                    fclose ( stream );

                    // gnuplot:
                    // base=0.2
                    // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)
                    double base = strtod ( argv[1], NULL );
                    double prob = -1.0 * log(size[0]/base) / log(base)
                                + log(class[0] + 1.0) / log(base);

                    double weight = (class[1] + 1.0) / (double)size[1];

                    newtime = nwIdleTime * weight * prob;

                    if ( newtime >= myIdleTime ) {
                        nwIdleTime = newtime;

                        class[1] = addValue ( &groups[1], &newtime );

                        FILE * stream = fopen ( seed[1], "a" );
                        fwrite ( &newtime, sizeof ( unsigned int ), 1, stream );
                        fclose ( stream );

                        XSyncIntToValue ( &value[1], newtime );
                        attributes[0].trigger.wait_value = value[1];
                    }

                }

            } else {
                attributes[0].trigger.test_type = XSyncNegativeComparison;
            }

            XSyncChangeAlarm ( dpy, alarm[0], flags, &attributes[0] );
            lastEventTime = alarmEvent->time;
        }

    }

    dumpGroup ( &groups[0], seed[0] );
    finalizeGroup ( &groups[0] );

    dumpGroup ( &groups[1], seed[1] );
    finalizeGroup ( &groups[1] );

    return 0;

}

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
            | XSyncValueLow32 ( *value )
           );
}

static void installSignalHandler ( int nsignals, int * signals ) {
    int i;
    struct sigaction sa;

    memset ( &sa, 0, sizeof ( struct sigaction ) );
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = signalHandler;

    for ( i = 0; i < nsignals; i++ ) sigaction ( signals[i],  &sa, NULL );
}

static void signalHandler ( int sig, siginfo_t * siginfo, void * context ) {
    int g;

    fprintf ( stderr, "Caught signal %d\n", sig );

    switch (sig) {
        case SIGUSR1:
            for ( g = 0; g < globalSignalData.ngroups; g++ ) {
                printGroup ( &(globalSignalData.groups[g]) );
            }
            break;

        default:
            for ( g = 0; g < globalSignalData.ngroups; g++ ) {
                dumpGroup ( &(globalSignalData.groups[g])
                          ,   globalSignalData.seed[g]
                          );
            }
            exit ( EXIT_SUCCESS );
    }
}

static
void initGroups ( int (* init) (int)
                , int ngroups
                , group_t * group
                , int * size
                , const char ** seed
                ) {
    int i, k;

    for ( i = 0; i < ngroups; i++ ) {
        makeGroup ( &group[i], size[i] );
        for ( k = 0; k < size[i]; k++ ) {
            // group[0].cluster[k].mean = myIdleTime * k / (double)size[0];
            group[i].cluster[k].mean = init ( k / (double)size[i] );
        }
        seedGroup ( &group[i], seed[i] );
    }
}
