#include "signal.h"

#include <string.h>

void installSignalHandler
    ( int nsignals
    , int * signals
    , void (* signalHandler) (int, siginfo_t *, void *)
    ) {
    int i;
    struct sigaction sa;

    memset ( &sa, 0, sizeof ( struct sigaction ) );
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = signalHandler;

    for ( i = 0; i < nsignals; i++ ) sigaction ( signals[i],  &sa, NULL );
}
