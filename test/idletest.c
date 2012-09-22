#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

long XSyncValueToLong ( XSyncValue *value );

int main ( int argc, char ** argv ) {

    unsigned long flags = XSyncCACounter
                        | XSyncCAValueType
                        | XSyncCATestType
                        | XSyncCAValue
                        | XSyncCADelta
                        ;

    int idlecount = 0
      , i = 0
      , major = 0, minor = 0
      , ev_base = 0, err_base = 0
      , numCounter = 0
      , intIdletime = 1000
      , backoff = 1000
      ;

    XEvent xEvent;
    Time lastEventTime;
    XSyncValue delta, idletime;
    XSyncAlarmNotifyEvent * alarmEvent = NULL;

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

    XSyncIntToValue ( &delta, 0 );
    XSyncIntToValue ( &idletime, intIdletime );

    XSyncAlarmAttributes attributes;
    attributes.trigger.counter    = counter->counter;
    attributes.trigger.value_type = XSyncAbsolute;
    attributes.trigger.test_type  = XSyncPositiveComparison;
    attributes.trigger.wait_value = idletime;
    attributes.delta              = delta;


    XSyncAlarm alarm = XSyncCreateAlarm ( dpy, flags, &attributes );

    while ( 1 ) {
        XNextEvent ( dpy, &xEvent );

        fprintf ( stderr , "Event! " );

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( XSyncValueLessThan ( alarmEvent->counter_value
                                , alarmEvent->alarm_value
                                )
           ) {
            attributes.trigger.test_type = XSyncPositiveComparison;
            fprintf ( stderr
                    , "XSyncPositiveComparison; %lu; %lu\n"
                    , XSyncValueToLong ( &alarmEvent->alarm_value )
                    , XSyncValueToLong ( &alarmEvent->counter_value )
                    );
        } else {
            attributes.trigger.test_type = XSyncNegativeComparison;
            fprintf ( stderr
                    , "XSyncNegativeComparison; %lu; %lu\n"
                    , XSyncValueToLong ( &alarmEvent->alarm_value )
                    , XSyncValueToLong ( &alarmEvent->counter_value )
                    );
        }

        XSyncChangeAlarm ( dpy, alarm, flags, &attributes );
        lastEventTime = alarmEvent->time;

    }

    return 0;

}

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
            | XSyncValueLow32 ( *value )
           );
}
