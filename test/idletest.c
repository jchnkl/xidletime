#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <signal.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "kMeans.h"

typedef struct sigData
    { int ngroups
    ; group_t * groups
    ; const char ** groupFiles
    ;
    } sigData;

sigData globalSigData;

long XSyncValueToLong ( XSyncValue *value );

static void sigHandler ( int sig, siginfo_t * siginfo, void * context );

int main ( int argc, char ** argv ) {

    int i = 0, k = 0
      , major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , numCounter = 0
      , myIdleTime = strtol ( argv[2], NULL, 10 ) * 1000
      , nwIdleTime = myIdleTime
      ;

    struct sigaction sa;
    memset ( &sa, 0, sizeof ( struct sigaction ) );
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = sigHandler;
    sigaction ( SIGINT,  &sa, NULL );
    sigaction ( SIGTERM, &sa, NULL );
    sigaction ( SIGUSR1, &sa, NULL );

    const char * groupFiles[2];
    groupFiles[0] = argv[3]; // idleFile
    groupFiles[1] = argv[4]; // timeoutFile

    group_t groups[2];
    int idleGroupSize = 100;
    makeGroup ( &groups[0], idleGroupSize );

    for ( k = 0; k < idleGroupSize; k++ ) {
        groups[0].cluster[k].mean = myIdleTime * k / (double)idleGroupSize;
    }

    seedGroup ( &groups[0], groupFiles[0] );
    updateGroup ( &groups[0] );

    int timeoutGroupSize = 10;
    makeGroup ( &groups[1], timeoutGroupSize );
    groups[1].cmp_type = FILL;

    for ( k = 0; k < timeoutGroupSize; k++ ) {
        groups[1].cluster[k].mean = myIdleTime * k / (double)timeoutGroupSize;
    }

    seedGroup ( &groups[1], groupFiles[1] );

    memset ( &globalSigData, 0, sizeof ( sigData ) );
    globalSigData.ngroups = 2;
    globalSigData.groups = groups;
    globalSigData.groupFiles = groupFiles;

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

    char buf[256];

    int class[2];
    class[0] = idleGroupSize - 1;
    class[1] = timeoutGroupSize - 1;

    while ( 1 ) {
        memset ( buf, 0, 63 );
        XNextEvent ( dpy, &xEvent );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( alarmEvent->alarm == alarm[0] ) {
            strcat ( buf,  "alarm[0]: " );
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                strcat ( buf,  "Reset" );
                attributes[0].trigger.test_type = XSyncPositiveComparison;

                if ( lastEventTime != 0 && lastEventTime < alarmEvent->time ) {

                    unsigned int newtime = 0;
                    unsigned int time = alarmEvent->time - lastEventTime;

                    class[0] = addValue ( &groups[0], &time );

                    FILE * stream = fopen ( groupFiles[0], "a" );
                    fwrite ( &time, sizeof ( unsigned int ), 1, stream );
                    fclose ( stream );

                    // gnuplot:
                    // base=0.2
                    // plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)
                    double base = strtod ( argv[1], NULL );
                    double prob = -1.0 * log(idleGroupSize/base) / log(base)
                                + log(class[0] + 1.0) / log(base);

                    double weight = (class[1] + 1.0) / (double)timeoutGroupSize;

                    newtime = nwIdleTime * weight * prob;

                    char tmp[64];
                    snprintf ( tmp
                             , 64
                             , " [%i/100 = %.2f; w: %f; p: %f]"
                             , class[0]
                             , prob
                             , weight
                             , weight * prob
                             );
                    strcat ( buf, tmp );

                    // if ( nwIdleTime >= myIdleTime ) {
                    if ( newtime >= myIdleTime ) {
                        nwIdleTime = newtime;

                        class[1] = addValue ( &groups[1], &newtime );

                        FILE * stream = fopen ( groupFiles[1], "a" );
                        fwrite ( &newtime, sizeof ( unsigned int ), 1, stream );
                        fclose ( stream );

                        XSyncIntToValue ( &value[1], newtime );
                        attributes[0].trigger.wait_value = value[1];
                    }

                    snprintf ( tmp
                             , 64
                             // , " %i / %i = %.2f (%i) (%i)"
                             , " newtime: %-i; new timeout: %-i; timeout class: %-i"
                             // , groupsize
                             // , prob
                             , newtime
                             , nwIdleTime / 1000
                             , class[1]
                             );
                    strcat ( buf, tmp );

                }

            } else {
                strcat ( buf,  "Idle" );
                attributes[0].trigger.test_type = XSyncNegativeComparison;
            }
            XSyncChangeAlarm ( dpy, alarm[0], flags, &attributes[0] );

            if ( lastEventTime < alarmEvent->time ) {
                fprintf ( stderr, "%s\n", buf );
            }
            lastEventTime = alarmEvent->time;

        }

    }

    dumpGroup ( &groups[0], groupFiles[0] );
    finalizeGroup ( &groups[0] );

    dumpGroup ( &groups[1], groupFiles[1] );
    finalizeGroup ( &groups[1] );

    return 0;

}

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
            | XSyncValueLow32 ( *value )
           );
}

static void sigHandler ( int sig, siginfo_t * siginfo, void * context ) {
    int g;

    fprintf ( stderr, "Caught signal %d\n", sig );

    switch (sig) {
        case SIGUSR1:
            for ( g = 0; g < globalSigData.ngroups; g++ ) {
                printGroup ( &(globalSigData.groups[g]) );
            }
            break;

        default:
            for ( g = 0; g < globalSigData.ngroups; g++ ) {
                dumpGroup ( &(globalSigData.groups[g])
                          ,   globalSigData.groupFiles[g]
                          );
            }
            exit ( EXIT_SUCCESS );
    }
}
