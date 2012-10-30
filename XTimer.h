#ifndef __XTIMER_H
#define __XTIMER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>
#include <X11/extensions/scrnsaver.h>

typedef enum TimerStatusT { Idle=0, Reset } TimerStatusT;

typedef struct XTimerT
    { Display              * dpy           // ptr to display
    ; uint                   idletime      // timeout for idle alarm
    ;
    } XTimerT;

typedef struct XTimerCallbackT
    { TimerStatusT            status
    ; XTimerT               * xtimer
    ; void                  * data
    ; void ( * run ) ( struct XTimerCallbackT * )
    ;
    } XTimerCallbackT;

void initXTimer ( XTimerT * );

void runXTimer ( XTimerT *, XTimerCallbackT * );

uint getXIdleTime ( XTimerT * xtimer );

int setXIdleTime ( XTimerT * xtimer, uint idletime );

#endif
