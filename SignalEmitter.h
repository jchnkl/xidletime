#ifndef __SIGNALEMITTER_H
#define __SIGNALEMITTER_H

typedef void Config;

typedef struct SignalEmitter
    { Config * config
    ; int  ( * emitSignal ) ( struct SignalEmitter *se, char * name, void * args )
    ;
    } SignalEmitter;


#endif
