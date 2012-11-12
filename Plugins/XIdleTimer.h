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
    ; uint               idletime      // timeout for idle alarm
    ; uint               suspend       // do not emit idle events
    ; TimerStatusT       status
    ;
    } XTimerT;

uint getXIdleTime ( void );

void setXIdleTime ( uint idletime );

void * xIdleTimerSource ( EventSourceT * );

void xIdleTimerSink ( EventSinkT *, EventSourceT * );

void suspendIdleTimer ( int suspend );

#endif
