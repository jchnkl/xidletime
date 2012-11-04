#ifndef __DEQUE_H
#define __DEQUE_H

#include <pthread.h>

typedef void ElementT;

typedef struct DequeContainerT
    {        ElementT   * element
    ; struct DequeContainerT * next
    ; struct DequeContainerT * prev
    ;
    } DequeContainerT;

typedef struct DequeT
    { DequeContainerT      * head
    ; DequeContainerT      * last
    ; pthread_mutex_t   dequelock
    ; unsigned int      dynamic
    ;
    } DequeT;

DequeT * makeDeque ( DequeT * );

void destroyDeque ( DequeT *, void ( * ) (ElementT *) );

int isEmpty ( DequeT * );

void pushHead ( DequeT *, ElementT *, ElementT * ( * ) ( void ) );

void pushLast ( DequeT *, ElementT *, ElementT * ( * ) ( void ) );

ElementT * popHead ( DequeT * );

ElementT * popLast ( DequeT * );

void iterateDequeWith ( DequeT *, void ( * ) ( ElementT * ) );

void reverseIterateDequeWith ( DequeT *, void ( * ) ( ElementT * ) );

#endif
