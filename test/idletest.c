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

typedef struct groupData
    { int (* init) (int)
    ; int ngroups
    ; group_t * group
    ; int * size
    ; const char ** seed
    ;
    } groupData;

static
void initGroups ( groupData * gd );

typedef struct alarmData
    { Display * dpy
    ; ulong * flags
    ; XSyncAlarmAttributes * attributes
    ; XSyncAlarm * alarm
    ; int * major
    ; int * minor
    ; int * ev_base
    ; int * err_base
    ; int * idletime
    ;
    } alarmData;

static
void initAlarm ( alarmData * ad );

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

    groupData gd; memset ( &gd, 0, sizeof ( groupData ) );
    gd.init = initMeans;
    gd.ngroups = 2;
    gd.group = group;
    gd.size = size;
    gd.seed = seed;
    initGroups ( &gd );

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

    XSyncAlarm alarm;
    XSyncAlarmAttributes attributes;

    alarmData ad; memset ( &ad, 0, sizeof ( alarmData ) );
    ad.flags = &flags;
    ad.attributes = &attributes;
    ad.alarm = &alarm;
    ad.major = &major;
    ad.minor = &minor;
    ad.ev_base = &ev_base;
    ad.err_base = &err_base;
    ad.idletime = &myIdleTime;

    initAlarm ( &ad );

    int class[2];
    class[0] = size[0] - 1;
    class[1] = size[1] - 1;

    while ( 1 ) {
        XNextEvent ( ad.dpy, &xEvent );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        XSyncAlarmNotifyEvent * alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( alarmEvent->alarm == alarm ) {
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                fprintf ( stderr, "Reset\n" );
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
                fprintf ( stderr, "Idle\n" );
                attributes.trigger.test_type = XSyncNegativeComparison;
            }

            XSyncChangeAlarm ( ad.dpy, alarm, flags, &attributes );
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
void initGroups ( groupData * gd ) {
    int i, k;

    for ( i = 0; i < gd->ngroups; i++ ) {
        makeGroup ( &(gd->group[i]), gd->size[i] );
        for ( k = 0; k < gd->size[i]; k++ ) {
            // group[0].cluster[k].mean = myIdleTime * k / (double)size[0];
            gd->group[i].cluster[k].mean = gd->init ( k / (double)(gd->size[i]) );
        }
        seedGroup ( &(gd->group[i]), gd->seed[i] );
    }
}

static
void initAlarm ( alarmData * ad ) {
    int i, listCount = 0;
    XSyncSystemCounter * sysCounter = NULL, * counter = NULL;
    XSyncValue value[2];

    ad->dpy = XOpenDisplay ("");
    Window root = DefaultRootWindow ( ad->dpy );
    XSelectInput ( ad->dpy, root, XSyncAlarmNotifyMask );
    XSyncInitialize ( ad->dpy, ad->major, ad->minor );
    XSyncQueryExtension ( ad->dpy , ad->ev_base , ad->err_base );

    sysCounter = XSyncListSystemCounters ( ad->dpy, &listCount );

    for ( i = 0; i < listCount; i++ ) {
        if ( 0 == strcmp ( sysCounter[i].name, "IDLETIME" ) ) {
            counter = &sysCounter[i];
        }
    }

    XSyncIntToValue ( &value[0], 0 );
    XSyncIntToValue ( &value[1], *(ad->idletime) );

    ad->attributes->trigger.counter    = counter->counter;
    ad->attributes->trigger.value_type = XSyncAbsolute;
    ad->attributes->trigger.test_type  = XSyncPositiveComparison;
    ad->attributes->trigger.wait_value = value[1];
    ad->attributes->delta              = value[0];

    *(ad->alarm) = XSyncCreateAlarm ( ad->dpy, *(ad->flags), ad->attributes );
}
