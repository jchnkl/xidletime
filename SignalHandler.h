#ifndef __SIGNALHANDLER_H
#define __SIGNALHANDLER_H

#include <signal.h>

void installSignalHandler
    ( int nsignals
    , int * signals
    , void (* signalHandler) (int, siginfo_t *, void *)
    );

#endif
