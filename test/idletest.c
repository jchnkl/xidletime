#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "kMeans.h"

typedef unsigned  int uint;
typedef unsigned long ulong;

typedef struct signalData
    { int ngroups
    ; group_t * group
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

static
XSyncAlarm initAlarm ( Display ** dpy
                     , ulong * flags
                     , XSyncAlarmAttributes * attributes
                     , int * major
                     , int * minor
                     , int * ev_base
                     , int * err_base
                     , int * idletime
                     );

int main ( int argc, char ** argv ) {

    int major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , myIdleTime = strtol ( argv[2], NULL, 10 ) * 1000
      , nwIdleTime = myIdleTime
      ;

    const char * seed[2];
    seed[0] = argv[3]; // idleFile
    seed[1] = argv[4]; // timeoutFile

    group_t group[2];
    int size[] = { 100, 10 };
    int initMeans ( int v ) { return myIdleTime * v; }
    initGroups ( initMeans, 2, group, size, seed );

    memset ( &globalSignalData, 0, sizeof ( signalData ) );
    globalSignalData.ngroups = 2;
    globalSignalData.group = group;
    globalSignalData.seed = seed;

    int signals[] = { SIGINT, SIGTERM, SIGUSR1 };
    installSignalHandler ( 3, signals );

    XEvent xEvent;
    Time lastEventTime = 0;

    ulong flags = XSyncCACounter
                | XSyncCAValueType
                | XSyncCATestType
                | XSyncCAValue
                | XSyncCADelta
                ;
    Display * dpy = NULL;
    XSyncAlarm alarm[2];
    XSyncAlarmAttributes attributes;
    alarm[0] = initAlarm ( &dpy
                         , &flags
                         , &attributes
                         , &major
                         , &minor
                         , &ev_base
                         , &err_base
                         , &myIdleTime
                         );

    if ( dpy == NULL ) exit ( EXIT_FAILURE );

    int class[2];
    class[0] = size[0] - 1;
    class[1] = size[1] - 1;

    while ( 1 ) {
        XNextEvent ( dpy, &xEvent );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        XSyncAlarmNotifyEvent * alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( alarmEvent->alarm == alarm[0] ) {
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                attributes.trigger.test_type = XSyncPositiveComparison;

                if ( lastEventTime != 0 && lastEventTime < alarmEvent->time ) {

                    uint newtime = 0;
                    uint time = alarmEvent->time - lastEventTime;

                    class[0] = addValue ( &group[0], &time );

                    FILE * stream = fopen ( seed[0], "a" );
                    fwrite ( &time, sizeof ( uint ), 1, stream );
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

                        class[1] = addValue ( &group[1], &newtime );

                        FILE * stream = fopen ( seed[1], "a" );
                        fwrite ( &newtime, sizeof ( uint ), 1, stream );
                        fclose ( stream );

                        XSyncValue value;
                        XSyncIntToValue ( &value, newtime );
                        attributes.trigger.wait_value = value;
                    }

                }

            } else {
                attributes.trigger.test_type = XSyncNegativeComparison;
            }

            XSyncChangeAlarm ( dpy, alarm[0], flags, &attributes );
            lastEventTime = alarmEvent->time;
        }

    }

    dumpGroup ( &group[0], seed[0] );
    finalizeGroup ( &group[0] );

    dumpGroup ( &group[1], seed[1] );
    finalizeGroup ( &group[1] );

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
                printGroup ( &(globalSignalData.group[g]) );
            }
            break;

        default:
            for ( g = 0; g < globalSignalData.ngroups; g++ ) {
                dumpGroup ( &(globalSignalData.group[g])
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

static
XSyncAlarm initAlarm ( Display ** dpy
                     , ulong * flags
                     , XSyncAlarmAttributes * attributes
                     , int * major
                     , int * minor
                     , int * ev_base
                     , int * err_base
                     , int * idletime
                     ) {
    int i, listCount = 0;
    XSyncSystemCounter * sysCounter = NULL, * counter = NULL;
    XSyncValue value[2];

    *dpy = XOpenDisplay ("");
    Window root = DefaultRootWindow ( *dpy );
    XSelectInput ( *dpy, root, XSyncAlarmNotifyMask );
    XSyncInitialize ( *dpy, major, minor );
    XSyncQueryExtension ( *dpy , ev_base , err_base );

    sysCounter = XSyncListSystemCounters ( *dpy, &listCount );

    for ( i = 0; i < listCount; i++ ) {
        if ( 0 == strcmp ( sysCounter[i].name, "IDLETIME" ) ) {
            counter = &sysCounter[i];
        }
    }

    XSyncIntToValue ( &value[0], 0 );
    XSyncIntToValue ( &value[1], *idletime );

    attributes->trigger.counter    = counter->counter;
    attributes->trigger.value_type = XSyncAbsolute;
    attributes->trigger.test_type  = XSyncPositiveComparison;
    attributes->trigger.wait_value = value[1];
    attributes->delta              = value[0];

    return XSyncCreateAlarm ( *dpy, *flags, attributes );
}
