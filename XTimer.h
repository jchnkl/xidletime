#ifndef __XTIMER_H
#define __XTIMER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "Callback.h"

typedef enum TimerStatusT { Idle=0, Reset } TimerStatusT;

typedef struct XTimerT
    { Display              * dpy           // ptr to display
    ; int                    major         // major version
    ; int                    minor         // minor version
    ; int                    ev_base       // offset for event ptr
    ; int                    err_base      // offset for error
    ; int                    idletime      // timeout for idle alarm
    ; ulong                  flags         // flags for attributes
    ; Time                   lastEventTime // time of last event
    ; XSyncAlarm             alarm         // alarm id
    ; XSyncAlarmAttributes * attributes    // attributes
    ;
    } XTimerT;

typedef struct IdleTimerCallbackT
    { TimerStatusT            status
    ; XTimerT         * itd
    ; XSyncAlarmNotifyEvent * xsane
    ; void                  * data
    ;
    } IdleTimerCallbackT;

long XSyncValueToLong ( XSyncValue *value );

void initXTimer ( XTimerT * );

void runXTimer ( XTimerT *, CallbackT * );

#endif
