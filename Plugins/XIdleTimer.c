#include "XIdleTimer.h"

#include <stdio.h>
#include <string.h>

static void initXTimer ( XTimerT * xtimer ) {
    if ( xtimer->dpy == NULL ) xtimer->dpy = XOpenDisplay ("");
    XSetScreenSaver ( xtimer->dpy, xtimer->idletime / 1000, 0, False, False );
}

static void runXTimer ( XTimerT * xtimer, XTimerCallbackT * xtcallback ) {

    int xssEvBase, xssErrBase;

    XEvent xEvent;
    XScreenSaverNotifyEvent * xssEvent = (XScreenSaverNotifyEvent *) &xEvent;

    Window root = DefaultRootWindow ( xtimer->dpy );

    XScreenSaverQueryExtension ( xtimer->dpy, &xssEvBase, &xssErrBase );
    XScreenSaverSelectInput ( xtimer->dpy, root, ScreenSaverNotifyMask );

    xtcallback->xtimer = xtimer;

    while ( 1 ) {
        XNextEvent ( xtimer->dpy, &xEvent );
        if ( xEvent.type != xssEvBase + ScreenSaverNotify ) continue;


        if ( xssEvent->state == ScreenSaverOn ) {
            xtcallback->status = Idle;
            xtcallback->run ( xtcallback );
        } else {
            xtcallback->status = Reset;
            xtcallback->run ( xtcallback );
        }

    }
}

uint getXIdleTime ( XTimerT * xtimer ) {
    return xtimer->idletime;
}

int setXIdleTime ( XTimerT * xtimer, uint idletime ) {
    if ( xtimer == NULL ) return -1;
    xtimer->idletime = idletime;
    XSetScreenSaver ( xtimer->dpy, xtimer->idletime / 1000, 0, False, False );
    return 0;
}

void * xIdleTimerSource ( EventSourceT * es ) {
    XTimerT xtimer; memset ( &xtimer, 0, sizeof ( XTimerT ) );
    XTimerCallbackT xtcb; memset ( &xtcb, 0, sizeof ( XTimerCallbackT ) );

    void cb ( XTimerCallbackT * xtc ) {
        es->private = (void *)xtc;
        es->eq->queueEvent ( es->eq, es );
    }

    xtcb.run = cb;

    xtimer.idletime = 7000;

    initXTimer ( &xtimer );

    runXTimer ( &xtimer, &xtcb );

    return NULL;
}

void xIdleTimerSink ( EventSinkT * snk, EventSourceT * src ) {
    if ( ((XTimerCallbackT *)src->private)->status == Idle ) {
        fprintf ( stdout, "Idle\n" );
    } else {
        fprintf ( stdout, "Reset\n" );
    }
}
