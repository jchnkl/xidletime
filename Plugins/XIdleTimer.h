#ifndef __XIDLETIMER_H
#define __XIDLETIMER_H

#include "../EventQueue.h"

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xproto.h>
#include <xcb/screensaver.h>

typedef enum TimerStatusT { Idle=0, Reset } TimerStatusT;

typedef struct XTimerT
    { xcb_connection_t * c
    ; uint                   idletime      // timeout for idle alarm
    ;
    } XTimerT;

typedef struct XTimerCallbackT
    { TimerStatusT   status
    ; XTimerT      * xtimer
    ; void ( * run ) ( struct XTimerCallbackT * )
    ;
    } XTimerCallbackT;

uint getXIdleTime ( void );

void setXIdleTime ( uint idletime );

void * xIdleTimerSource ( EventSourceT * );

void xIdleTimerSink ( EventSinkT *, EventSourceT * );

#endif
