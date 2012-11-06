#include "XIdleTimer.h"

#include <stdio.h>
#include <string.h>

static XTimerT staticXTimer;
static XTimerCallbackT staticXTCB;

static void runXTimer ( void ) {
    int xssEvBase, xssErrBase;

    XEvent xEvent;
    XScreenSaverNotifyEvent * xssEvent = (XScreenSaverNotifyEvent *) &xEvent;

    Window root = DefaultRootWindow ( staticXTimer.dpy );

    XScreenSaverQueryExtension ( staticXTimer.dpy, &xssEvBase, &xssErrBase );
    XScreenSaverSelectInput ( staticXTimer.dpy, root, ScreenSaverNotifyMask );

    while ( 1 ) {
        XNextEvent ( staticXTimer.dpy, &xEvent );
        if ( xEvent.type != xssEvBase + ScreenSaverNotify ) continue;


        if ( xssEvent->state == ScreenSaverOn ) {
            staticXTCB.status = Idle;
        } else {
            staticXTCB.status = Reset;
        }

        staticXTCB.run ( &staticXTCB );
    }
}

uint getXIdleTime ( void ) {
    return staticXTimer.idletime;
}

void setXIdleTime ( uint idletime ) {
    staticXTimer.idletime = idletime;
    XSetScreenSaver ( staticXTimer.dpy, staticXTimer.idletime / 1000, 0, False, False );
}

void * xIdleTimerSource ( EventSourceT * src ) {

    if ( src->private == NULL ) {
        memset ( &staticXTimer, 0, sizeof ( XTimerT ) );
        memset ( &staticXTCB, 0, sizeof ( XTimerCallbackT ) );

        void getConfig ( PublicConfigT * pc ) {
            if ( pc->dpy == NULL ) pc->dpy = XOpenDisplay ("");
            staticXTimer.dpy = pc->dpy;
            staticXTimer.idletime = pc->options->idletime;
        }
        withPublicConfig ( src->public, getConfig );

        XSetScreenSaver ( staticXTimer.dpy
                        , staticXTimer.idletime / 1000
                        , 0
                        , False
                        , False
                        );

        src->private = (void *)&staticXTCB;
    }

    staticXTCB.xtimer = &staticXTimer;

    void cb ( XTimerCallbackT * xtc ) {
        src->eq->queueEvent ( src->eq, src );
    }

    staticXTCB.run = cb;

    runXTimer();

    return NULL;
}

void xIdleTimerSink ( EventSinkT * snk, EventSourceT * src ) {
    if ( staticXTCB.status == Idle ) {
        fprintf ( stdout, "Idle\n" );
    } else {
        fprintf ( stdout, "Reset\n" );
    }
}
