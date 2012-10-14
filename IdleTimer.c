#include "IdleTimer.h"

#include <string.h>

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
            | XSyncValueLow32 ( *value )
           );
}

void initIdleTimer ( IdleTimerData * itd ) {
    int i, listCount = 0;
    XSyncSystemCounter * sysCounter = NULL, * counter = NULL;
    XSyncValue value[2];

    itd->dpy = XOpenDisplay ("");
    Window root = DefaultRootWindow ( itd->dpy );
    XSelectInput ( itd->dpy, root, XSyncAlarmNotifyMask );
    XSyncInitialize ( itd->dpy, &(itd->major), &(itd->minor) );
    XSyncQueryExtension ( itd->dpy , &(itd->ev_base), &(itd->err_base) );

    sysCounter = XSyncListSystemCounters ( itd->dpy, &listCount );

    for ( i = 0; i < listCount; i++ ) {
        if ( 0 == strcmp ( sysCounter[i].name, "IDLETIME" ) ) {
            counter = &sysCounter[i];
        }
    }

    XSyncIntToValue ( &value[0], 0 );
    XSyncIntToValue ( &value[1], itd->idletime );

    itd->attributes->trigger.counter    = counter->counter;
    itd->attributes->trigger.value_type = XSyncAbsolute;
    itd->attributes->trigger.test_type  = XSyncPositiveComparison;
    itd->attributes->trigger.wait_value = value[1];
    itd->attributes->delta              = value[0];

    itd->alarm = XSyncCreateAlarm ( itd->dpy, itd->flags, itd->attributes );
}

