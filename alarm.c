#include "alarm.h"

#include <string.h>

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
