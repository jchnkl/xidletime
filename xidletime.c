#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "alarm.h"
#include "group.h"
#include "signal.h"
#include "kmeans.h"

typedef unsigned  int uint;
typedef unsigned long ulong;

GroupData globalGroupData;

static void signalHandler ( int, siginfo_t *, void * );

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
    cmp_type_t comp[] = { MEAN, FILL };
    int initMeans ( int m, int s ) {
        return (int)((double)myIdleTime * (double)m / (double)s);
    }

    memset ( &globalGroupData, 0, sizeof ( GroupData ) );
    globalGroupData.init = initMeans;
    globalGroupData.ngroups = 2;
    globalGroupData.group = group;
    globalGroupData.size = size;
    globalGroupData.comp = comp;
    globalGroupData.seed = seed;
    initGroups ( &globalGroupData );

    int signals[] = { SIGINT, SIGTERM, SIGUSR1 };
    installSignalHandler ( 3, signals, signalHandler );

    ulong flags = XSyncCACounter
                | XSyncCAValueType
                | XSyncCATestType
                | XSyncCAValue
                | XSyncCADelta
                ;

    XSyncAlarm alarm;
    XSyncAlarmAttributes attributes;

    AlarmData ad; memset ( &ad, 0, sizeof ( AlarmData ) );
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

    XEvent xEvent;
    XSyncAlarmNotifyEvent * alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;
    Time lastEventTime = 0;

    while ( 1 ) {
        XNextEvent ( ad.dpy, &xEvent );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        if ( alarmEvent->alarm == alarm ) {
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                fprintf ( stderr, "Reset\n" );
                attributes.trigger.test_type = XSyncPositiveComparison;

                if ( lastEventTime != alarmEvent->time ) {

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

#ifdef DEBUG
                    fprintf ( stderr
                            , "time: %u\tclass[0]: %i\tclass[1]: %i\nprob: %f\tweight: %f\tnewtime: %i\n"
                            , time
                            , class[0]
                            , class[1]
                            , prob
                            , weight
                            , newtime
                            );
#endif

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
                if ( lastEventTime != alarmEvent->time )
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

static void signalHandler ( int sig, siginfo_t * siginfo, void * context ) {
    int g;

#ifdef DEBUG
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
