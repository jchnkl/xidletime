#ifndef __ALARM_H
#define __ALARM_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

typedef struct alarmData
    { Display * dpy                     // ptr to display
    ; ulong * flags                     // flags for attributes
    ; XSyncAlarmAttributes * attributes // attributes
    ; XSyncAlarm * alarm                // alarm id
    ; int * major                       // major version
    ; int * minor                       // minor version
    ; int * ev_base                     // offset for event ptr
    ; int * err_base                    // offset for error
    ; int * idletime                    // timeout for idle alarm
    ;
    } alarmData;

void initAlarm ( alarmData * ad );

#endif
