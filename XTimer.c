#include "XTimer.h"

#include <string.h>

void initXTimer ( XTimerT * xtimer ) {
    xtimer->dpy = XOpenDisplay ("");
    XSetScreenSaver ( xtimer->dpy, xtimer->idletime / 1000, 0, False, False );
}

void runXTimer ( XTimerT * xtimer, XTimerCallbackT * xtcallback ) {

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
