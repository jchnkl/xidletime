#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "XConfig.h"

unsigned int idletime      = 2000;
const char * busName       = "org.IdleTime";
const char * objectPath    = "/org/IdleTime";
const char * interfaceName = "org.IdleTime";

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
           | XSyncValueLow32 ( *value )
           );
}

int main ( int argc, char ** argv ) {

    int ret = 0, major = 0, minor = 0;
    int ev_base = 0, err_base = 0;

    DBusConnection * conn = NULL;
    conn = dbusInit ( &ret );
    if ( ret == -1 || conn == NULL ) { exit ( EXIT_FAILURE ); }


    ret = XSelectInput ( dpy, root, XSyncAlarmNotifyMask );
    if ( ret == 0 ) { exit ( EXIT_FAILURE ); }

    ret = XSyncInitialize ( dpy, &major, &minor );
    if ( ret == 0 ) { exit ( EXIT_FAILURE ); }
    printf ( "major: %i, minor: %i\n", major, minor );

    XSyncQueryExtension ( dpy, &ev_base, &err_base );
    printf ( "ev_base: %i, err_base: %i\n", ev_base, err_base );

    int i = 0, nxssc = 0;
    XSyncSystemCounter * xssc = XSyncListSystemCounters ( dpy, &nxssc );

    XSyncSystemCounter * idlecounter = NULL;
    for (i = 0; i < nxssc; i++) {
        if ( 0 == strcmp ( xssc[i].name, "IDLETIME" ) ) {
            printf ( "%s\n", xssc[i].name );
            idlecounter = &xssc[i];
        }
    }

    if ( idlecounter != NULL ) {

        unsigned long flags = XSyncCACounter
                            | XSyncCAValueType
                            | XSyncCATestType
                            | XSyncCAValue
                            | XSyncCADelta
                            ;

        XEvent xev;
        XSyncValue delta, timeout;

        XSyncAlarmAttributes xsaa;

        XSyncIntToValue (&delta, 0);
        XSyncIntToValue (&timeout, idletime);

        xsaa.trigger.counter    = idlecounter->counter;
        xsaa.trigger.value_type = XSyncAbsolute;
        xsaa.trigger.test_type  = XSyncPositiveComparison;
        xsaa.trigger.wait_value = timeout;
        xsaa.delta              = delta;

        XSyncAlarm setAlarm ( Bool reset, XSyncValue to ) {
            if ( reset ) {
                xsaa.trigger.test_type  = XSyncPositiveComparison;
                xsaa.trigger.wait_value = to;
            } else {
                xsaa.trigger.test_type  = XSyncNegativeComparison;
                xsaa.trigger.wait_value = to;
            }
            return XSyncCreateAlarm ( dpy, flags, &xsaa );
        }

        void changeAlarm ( XSyncAlarm * alarm, Bool reset, XSyncValue to ) {
            if ( reset ) {
                xsaa.trigger.test_type  = XSyncPositiveComparison;
                xsaa.trigger.wait_value = to;
            } else {
                xsaa.trigger.test_type  = XSyncNegativeComparison;
                xsaa.trigger.wait_value = to;
            }
            XSyncChangeAlarm ( dpy, *alarm, flags, &xsaa );
        }

        Time last;
        XSyncAlarm alarm = setAlarm ( True, timeout );
        while ( 1 ) {
            XNextEvent ( dpy, &xev );

            if ( xev.type == ev_base + XSyncAlarmNotify ) {
                XSyncAlarmNotifyEvent * xane = (XSyncAlarmNotifyEvent *) &xev;

                if ( last != xane->time ) {
                    fprintf ( stderr, "last!=xane->time\n" );
                }

                Bool lessThan = XSyncValueLessThan ( xane->counter_value
                                                   , xane->alarm_value
                                                   );

                if ( lessThan && last != xane->time ) {
                    fprintf ( stderr, "XSyncValueLessThan TRUE\n" );
                    xsaa.trigger.test_type = XSyncPositiveComparison;
                    dbusSendSignal ( conn, "Reset" );
                } else if ( last != xane->time ) {
                    fprintf ( stderr, "XSyncValueLessThan FALSE\n" );
                    xsaa.trigger.test_type = XSyncNegativeComparison;
                    dbusSendSignal ( conn, "Idle" );
                }

                XSyncChangeAlarm ( dpy, alarm, flags, &xsaa );
                last = xane->time;

                fprintf ( stderr
                        , "alarm; time: %i, serial: %lu, counter_value: %lu, alarm_value: %lu\n"
                        , (int)xane->time
                        , xane->serial
                        , XSyncValueToLong ( &(xane->counter_value) )
                        , XSyncValueToLong ( &(xane->alarm_value) )
                        );

            }

        }
    }

    XSyncFreeSystemCounterList ( xssc );



    return 0;
}
    XConfig xconfig;
    memset ( &xconfig, 0, sizeof ( XConfig ) );



    if ( -1 == initXConfig ( &xconfig ) ) { goto exit; }


    }

    finalizeXConfig ( &xconfig );
    return 0;

}
