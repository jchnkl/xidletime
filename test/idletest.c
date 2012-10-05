#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "kMeans.h"

long XSyncValueToLong ( XSyncValue *value );

int main ( int argc, char ** argv ) {

    unsigned long flags = XSyncCACounter
                        | XSyncCAValueType
                        | XSyncCATestType
                        | XSyncCAValue
                        | XSyncCADelta
                        ;

    int i = 0
      , major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , numCounter = 0
      , myIdleTime = strtol ( argv[1], NULL, 10 ) * 1000
      , nwIdleTime = myIdleTime
      ;

    char groupFile = argv[1];

    int groupsize = 100;
    group_t group;
    makeGroup ( &group, groupsize, groupFile );

    for ( int k = 0; k < groupsize; k++ ) {
        group.cluster[k].mean = myIdleTime * k / (double)groupsize;
    }

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

    char buf[64];

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

                    int    class = addValue ( &group, &time ) + 1;

                    FILE * stream = fopen ( groupFile, "a" );
                    fwrite ( value, sizeof ( unsigned int ), 1, stream );
                    fclose ( stream );

                    if ( class < 50 ) {
                        // bc:
                        // for (i=50; i<=100; i=i+5) {
                        //  r = 1 + sqrt ( ( i - 50 ) * 2 / 100 );
                        //  print i, ": ", r, "\n";
                        // }
                        int newtime = nwIdleTime * ( 1.0 + sqrt ( ( 50 - class ) * 2.0 / 100.0 ) );
                        if ( newtime >= myIdleTime ) nwIdleTime = newtime;
                        char tmp[16];
                        snprintf ( tmp
                                 , 16
                                 , " [p<50: %.2f]"
                                 , ( 1.0 + sqrt ( ( 50 - class ) * 2.0 / 100.0 ) )
                                 );
                        strcat ( buf, tmp );
                    } else {
                        // bc:
                        // for (i=50; i>=0; i=i-5) {
                        // r = 1 + sqrt ( ( 50 - i ) * 2 / 100 );
                        // print i, ": ", r, "\n";
                        // }
                        int newtime = nwIdleTime / ( 1.0 + sqrt ( ( class - 50 ) * 2.0 / 100.0 ) );
                        if ( newtime >= myIdleTime ) nwIdleTime = newtime;
                        char tmp[16];
                        snprintf ( tmp
                                 , 16
                                 , " [p>50: %.2f]"
                                 , ( 1.0 + sqrt ( ( class - 50 ) * 2.0 / 100.0 ) )
                                 );
                        strcat ( buf, tmp );
                    }

                    if ( nwIdleTime >= myIdleTime ) {
                        XSyncIntToValue ( &value[1], nwIdleTime );
                        attributes[0].trigger.wait_value = value[1];
                    }

                    int valueCount = 0;
                    for ( int c = 0; c < group.size; c++ ) {
                        valueCount += group.cluster[c].fillcount;
                    }

                    char tmp[32];
                    snprintf ( tmp
                             , 32
                             // , " %i / %i = %.2f (%i) (%i)"
                             , " %i; %i; %i"
                             , class
                             // , groupsize
                             // , prob
                             , nwIdleTime / 1000
                             , valueCount
                             );
                    strcat ( buf, tmp );

                    /*
                    fprintf ( stderr
                            , "added %u to class %i; probabilty: %f\n"
                            , time
                            , class
                            , (double)class / (double)groupsize
                            );
                    printMeans ( &group );
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
                int degrade = nwIdleTime - myIdleTime;

                if ( 0 && degrade >= myIdleTime ) {
                    nwIdleTime = degrade;
                    XSyncIntToValue ( &value[1], degrade );
                    attributes[0].trigger.wait_value = value[1];
                    attributes[0].trigger.test_type = XSyncPositiveComparison;
                    XSyncChangeAlarm ( dpy, alarm[0], flags, &attributes[0] );
                    fprintf ( stderr, "degrading to %i\n", degrade );
                    fprintf ( stderr, "\n" );
                }

                attributes[1].trigger.test_type = XSyncPositiveComparison;
            } else {
                attributes[1].trigger.test_type = XSyncNegativeComparison;
            }
            XSyncChangeAlarm ( dpy, alarm[1], flags, &attributes[1] );

        }

    }

    finalizeGroup ( &group );

    return 0;

}

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
            | XSyncValueLow32 ( *value )
           );
}
