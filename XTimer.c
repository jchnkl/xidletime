#include "XTimer.h"

#include <string.h>

void initXTimer ( XTimerT * xtimer ) {
    int i, listCount = 0;
    XSyncSystemCounter * sysCounter = NULL, * counter = NULL;
    XSyncValue value[2];

    xtimer->dpy = XOpenDisplay ("");
    Window root = DefaultRootWindow ( xtimer->dpy );
    XSelectInput ( xtimer->dpy, root, XSyncAlarmNotifyMask );
    XSyncInitialize ( xtimer->dpy, &(xtimer->major), &(xtimer->minor) );
    XSyncQueryExtension ( xtimer->dpy , &(xtimer->ev_base), &(xtimer->err_base) );

    sysCounter = XSyncListSystemCounters ( xtimer->dpy, &listCount );

    for ( i = 0; i < listCount; i++ ) {
        if ( 0 == strcmp ( sysCounter[i].name, "IDLETIME" ) ) {
            counter = &sysCounter[i];
        }
    }

    XSyncIntToValue ( &value[0], 0 );
    XSyncIntToValue ( &value[1], xtimer->idletime );

    xtimer->attributes->trigger.counter    = counter->counter;
    xtimer->attributes->trigger.value_type = XSyncAbsolute;
    xtimer->attributes->trigger.test_type  = XSyncPositiveComparison;
    xtimer->attributes->trigger.wait_value = value[1];
    xtimer->attributes->delta              = value[0];

    xtimer->alarm =
        XSyncCreateAlarm ( xtimer->dpy, xtimer->flags, xtimer->attributes );
}

void runXTimer ( XTimerT * xtimer, CallbackT * callback ) {

    XEvent xEvent;
    XSyncAlarmNotifyEvent * alarmEvent = (XSyncAlarmNotifyEvent *) &xEvent;

    IdleTimerCallbackT * itc = (IdleTimerCallbackT *) callback->data;
    itc->xtimer   = xtimer;
    itc->xsane = alarmEvent;

    while ( 1 ) {
        XNextEvent ( xtimer->dpy, &xEvent );

        if ( xEvent.type != xtimer->ev_base + XSyncAlarmNotify ) continue;

        if ( alarmEvent->alarm == xtimer->alarm ) {
            if ( XSyncValueLessThan ( alarmEvent->counter_value
                                    , alarmEvent->alarm_value
                                    )
               ) {
                xtimer->attributes->trigger.test_type = XSyncPositiveComparison;
                if ( xtimer->lastEventTime != alarmEvent->time ) {
                    itc->status = Reset;
                    callback->run ( callback->data );
                }
            } else {
                xtimer->attributes->trigger.test_type = XSyncNegativeComparison;
                if ( xtimer->lastEventTime != alarmEvent->time ) {
                    itc->status = Idle;
                    callback->run ( callback->data );
                }
            }

            XSyncChangeAlarm ( xtimer->dpy
                             , xtimer->alarm
                             , xtimer->flags
                             , xtimer->attributes
                             );

            xtimer->lastEventTime = alarmEvent->time;
        }

    }
}

int setXIdleTime ( XTimerT * xtimer, uint idletime ) {
    if ( xtimer == NULL ) return -1;
    xtimer->idletime = idletime;
    XSyncValue value;
    XSyncIntToValue ( &value, idletime );
    xtimer->attributes->trigger.wait_value = value;
    return 0;
}
