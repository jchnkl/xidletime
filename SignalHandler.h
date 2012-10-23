#ifndef __SIGNALHANDLER_H
#define __SIGNALHANDLER_H

#include <signal.h>

static void * signalData;

void initializeSignalData ( void * sd ) { signalData = sd; }

void * getSignalData ( void ) { return signalData; }

void installSignalHandler
    ( int nsignals
    , int * signals
    , void (* signalHandler) (int, siginfo_t *, void *)
    );

#endif
