#ifndef __SIGNALEMITTER_H
#define __SIGNALEMITTER_H

typedef struct SignalEmitter
    { void * data
    ; int  ( * emitSignal ) ( struct SignalEmitter *se, char * name )
    ;
    } SignalEmitter;


#endif
