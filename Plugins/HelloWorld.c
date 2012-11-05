#include <stdio.h>
#include <unistd.h>

#include "HelloWorld.h"

static const char * hello = "Hello World!";

void * helloWorldSource ( EventSourceT * es ) {
    es->private = (void *)hello;
    while ( 1 ) {
        es->eq->queueEvent ( es->eq, es );
        sleep ( 1 );
    }
    return NULL;
}

void helloWorldSink ( EventSinkT * snk, EventSourceT * src ) {
    fprintf ( stderr, "%s\n", (char *)src->private );
}
