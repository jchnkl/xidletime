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

    unsigned long flags = XSyncCACounter
                        | XSyncCAValueType
                        | XSyncCATestType
                        | XSyncCAValue
                        | XSyncCADelta
                        ;

    int i = 0
      , k = 0
      , major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , numCounter = 0
      , myIdleTime = strtol ( argv[1], NULL, 10 ) * 1000
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
    groupFiles[0] = argv[2]; // idleFile
    groupFiles[1] = argv[3]; // timeoutFile

    group_t groups[2];
    int idleGroupSize = 100;
    makeGroup ( &groups[0], idleGroupSize, groupFiles[0] );

    for ( k = 0; k < idleGroupSize; k++ ) {
        groups[0].cluster[k].mean = myIdleTime * k / (double)idleGroupSize;
    }

    int timeoutGroupSize = 10;
    makeGroup ( &groups[1], timeoutGroupSize, groupFiles[1] );
    groups[1].cmp_type = FILL;

    for ( k = 0; k < timeoutGroupSize; k++ ) {
        groups[1].cluster[k].mean = myIdleTime * k / (double)timeoutGroupSize;
    }

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
    XSyncIntToValue ( &value[2], myIdleTime );

    XSyncAlarmAttributes attributes[2];

    attributes[0].trigger.counter    = counter->counter;
    attributes[0].trigger.value_type = XSyncAbsolute;
    attributes[0].trigger.test_type  = XSyncPositiveComparison;
    attributes[0].trigger.wait_value = value[1];
    attributes[0].delta              = value[0];

    attributes[1].trigger.counter    = counter->counter;
    attributes[1].trigger.value_type = XSyncAbsolute;
    attributes[1].trigger.test_type  = XSyncPositiveComparison;
    attributes[1].trigger.wait_value = value[2];
    attributes[1].delta              = value[0];


    alarm[0] = XSyncCreateAlarm ( dpy, flags, &attributes[0] );
    alarm[1] = XSyncCreateAlarm ( dpy, flags, &attributes[1] );

    char buf[256];

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


                    unsigned int time = alarmEvent->time - lastEventTime;

                    int class[2];

                    class[0] = addValue ( &groups[0], &time ) + 1;

                    FILE * stream = fopen ( groupFiles[0], "a" );
                    fwrite ( value, sizeof ( unsigned int ), 1, stream );
                    fclose ( stream );

                    if ( class[0] < 50 ) {
                        // bc:
                        // for (i=50; i<=100; i=i+5) {
                        //  r = 1 + sqrt ( ( i - 50 ) * 2 / 100 );
                        //  print i, ": ", r, "\n";
                        // }
                        unsigned int newtime = nwIdleTime * ( 1.0 + sqrt ( ( 50 - class[0] ) * 2.0 / 100.0 ) );

                        if ( newtime >= myIdleTime ) {

                            nwIdleTime = newtime;

                            class[1] = addValue ( &groups[1], &newtime );

                            FILE * stream = fopen ( groupFiles[1], "a" );
                            fwrite ( value, sizeof ( unsigned int ), 1, stream );
                            fclose ( stream );
                        }

                        char tmp[32];
                        snprintf ( tmp
                                 , 32
                                 , " [p<50: %i/100 = %.2f]"
                                 , class[0]
                                 , ( 1.0 + sqrt ( ( 50 - class[0] ) * 2.0 / 100.0 ) )
                                 );
                        strcat ( buf, tmp );
                    } else {
                        // bc:
                        // for (i=50; i>=0; i=i-5) {
                        // r = 1 + sqrt ( ( 50 - i ) * 2 / 100 );
                        // print i, ": ", r, "\n";
                        // }
                        unsigned int newtime = nwIdleTime / ( 1.0 + sqrt ( ( class[0] - 50 ) * 2.0 / 100.0 ) );

                        if ( newtime >= myIdleTime ) {

                            nwIdleTime = newtime;

                            class[1] = addValue ( &groups[1], &newtime );

                            FILE * stream = fopen ( groupFiles[1], "a" );
                            fwrite ( value, sizeof ( unsigned int ), 1, stream );
                            fclose ( stream );
                        }

                        char tmp[32];
                        snprintf ( tmp
                                 , 32
                                 , " [p>50: %i/100 = %.2f]"
                                 , class[0]
                                 , ( 1.0 + sqrt ( ( class[0] - 50 ) * 2.0 / 100.0 ) )
                                 );
                        strcat ( buf, tmp );
                    }

                    if ( nwIdleTime >= myIdleTime ) {
                        XSyncIntToValue ( &value[1], nwIdleTime );
                        attributes[0].trigger.wait_value = value[1];
                    }

                    int valueCount = 0;
                    for ( k = 0; k < groups[0].size; k++ ) {
                        valueCount += groups[0].cluster[k].fillcount;
                    }

                    char tmp[64];
                    snprintf ( tmp
                             , 64
                             // , " %i / %i = %.2f (%i) (%i)"
                             , " new timeout: %-i; timeout class: %-i"
                             // , groupsize
                             // , prob
                             , nwIdleTime / 1000
                             , class[1]
                             );
                    strcat ( buf, tmp );

                    /*
                    fprintf ( stderr
                            , "added %u to class[0] %i; probabilty: %f\n"
                            , time
                            , class[0]
                            , (double)class[0] / (double)groupsize
                            );
                    printMeans ( &groups[0] );
                    */

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

        } else if ( alarmEvent->alarm == alarm[1] ) {

            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                attributes[1].trigger.test_type = XSyncPositiveComparison;
            } else {
                attributes[1].trigger.test_type = XSyncNegativeComparison;
            }
            XSyncChangeAlarm ( dpy, alarm[1], flags, &attributes[1] );

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
