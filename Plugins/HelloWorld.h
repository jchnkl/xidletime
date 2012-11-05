#ifndef __HELLOWORLD_H
#define __HELLOWORLD_H

#include "../EventQueue.h"

void * helloWorldSource ( EventSourceT * );

void helloWorldSink ( EventSinkT *, EventSourceT * );

#endif
