#ifndef __IDLETIMER_H
#define __IDLETIMER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

typedef enum IdleT { Idle=0, Reset } IdleT;

typedef struct IdleTimerData
    { Display * dpy                     // ptr to display
    ; int major                         // major version
    ; int minor                         // minor version
    ; int ev_base                       // offset for event ptr
    ; int err_base                      // offset for error
    ; int idletime                      // timeout for idle alarm
    ; ulong flags                       // flags for attributes
    ; Time lastEventTime                // time of last event
    ; XSyncAlarm alarm                  // alarm id
    ; XSyncAlarmAttributes * attributes // attributes
    ;
    } IdleTimerData;

long XSyncValueToLong ( XSyncValue *value );

void initIdleTimer ( IdleTimerData * );

void runTimer
    ( IdleTimerData *
    , void (* cb) (IdleT, IdleTimerData *, XSyncAlarmNotifyEvent *, void *)
    , void *
    );

#endif
