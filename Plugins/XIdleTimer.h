#ifndef __XIDLETIMER_H
#define __XIDLETIMER_H

#include "../EventQueue.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

typedef enum TimerStatusT { Idle=0, Reset } TimerStatusT;

typedef struct XTimerT
    { Display              * dpy           // ptr to display
    ; uint                   idletime      // timeout for idle alarm
    ;
    } XTimerT;

typedef struct XTimerCallbackT
    { TimerStatusT   status
    ; XTimerT      * xtimer
    ; void ( * run ) ( struct XTimerCallbackT * )
    ;
    } XTimerCallbackT;

uint getXIdleTime ( XTimerT * xtimer );

int setXIdleTime ( XTimerT * xtimer, uint idletime );

void * xIdleTimerSource ( EventSourceT * );

void xIdleTimerSink ( EventSinkT *, EventSourceT * );

#endif
