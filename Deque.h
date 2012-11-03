#ifndef __DEQUE_H
#define __DEQUE_H

#include <pthread.h>

typedef void ElementT;

typedef struct ContainerT
    {        ElementT   * element
    ; struct ContainerT * next
    ; struct ContainerT * prev
    ;
    } ContainerT;

typedef struct DequeT
    { ContainerT      * head
    ; ContainerT      * last
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

void iterateWith ( DequeT *, void ( * ) ( ElementT * ) );

void reverseIterateWith ( DequeT *, void ( * ) ( ElementT * ) );

#endif
