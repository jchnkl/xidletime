#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <signal.h>
#include "kmeans.h"

void installSignalHandler
    ( int nsignals
    , int * signals
    , void (* signalHandler) (int, siginfo_t *, void *)
    );

#endif
