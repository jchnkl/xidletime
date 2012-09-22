#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

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

        if ( xEvent.type != ev_base + XSyncAlarmNotify ) continue;

        fprintf ( stderr
                , "XSyncAlarmNotify\n"
                );


        alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

        if ( XSyncValueLessThan ( alarmEvent->counter_value
                                , alarmEvent->alarm_value
                                )
           ) {

        } else {
        }

        XSyncChangeAlarm ( dpy, alarm, flags, &attributes );
        lastEventTime = alarmEvent->time;

    }

    return 0;

}
