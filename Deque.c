#include "Deque.h"

#include <stdlib.h>

static void lockDeque ( DequeT * dq ) {
    pthread_mutex_lock ( &(dq->dequelock) );
}

static void unlockDeque ( DequeT * dq ) {
    pthread_mutex_unlock ( &(dq->dequelock) );
}

DequeT * makeDeque ( DequeT * dq ) {
    if ( dq == NULL ) {
        dq = (DequeT *) calloc ( 1, sizeof ( DequeT ) );
        dq->dynamic = 1;
    } else {
        dq->dynamic = 0;
    }

    pthread_mutex_init ( &(dq->dequelock), NULL );
    dq->head = NULL;
    dq->last = NULL;

    return dq;
}

void destroyDeque ( DequeT * dq, void (* destroy) (ElementT *) ) {
    while ( ! isEmpty ( dq ) ) {
        ElementT * e = popLast ( dq );
        if ( e != NULL && destroy != NULL ) destroy ( e );
    }
    pthread_mutex_destroy ( &(dq->dequelock) );
    if ( dq->dynamic ) free ( dq );
}

int isEmpty ( DequeT * dq ) {
    lockDeque ( dq );
    int res = dq->head == NULL;
    unlockDeque ( dq );
    return res;
}

void pushHead ( DequeT * dq, ElementT * e, ElementT * (* make) ( void ) ) {
    DequeContainerT * c =
        (DequeContainerT *) calloc ( 1, sizeof ( DequeContainerT ) );
    if ( e == NULL && make != NULL ) e = make();
    c->element = e;

    lockDeque ( dq );

    if ( dq->head == NULL ) {
        dq->head = c; dq->last = c;
    } else {
        c->next = dq->head;
        dq->head->prev = c;
        dq->head = c;
    }

    unlockDeque ( dq );
}

void pushLast ( DequeT * dq, ElementT * e, ElementT * (* make) ( void ) ) {
    DequeContainerT * c =
        (DequeContainerT *) calloc ( 1, sizeof ( DequeContainerT ) );
    if ( e == NULL && make != NULL ) e = make();
    c->element = e;

    lockDeque ( dq );

    if ( dq->last == NULL ) {
        dq->head = c; dq->last = c;
    } else {
        dq->last->next = c;
        c->prev = dq->last;
        dq->last = c;
    }

    unlockDeque ( dq );
}

ElementT * popHead ( DequeT * dq ) {
    ElementT * e = NULL;
    lockDeque ( dq );

    if ( dq->head != NULL ) {
        e = dq->head->element;
        DequeContainerT * tmp = dq->head;
        dq->head = dq->head->next;
        if ( dq->head != NULL ) dq->head->prev = NULL;
        if ( dq->last == tmp ) dq->last = dq->head;
        free ( tmp );
    }

    unlockDeque ( dq );
    return e;
}

ElementT * popLast ( DequeT * dq ) {
    ElementT * e = NULL;
    lockDeque ( dq );

    if ( dq->last != NULL ) {
        e = dq->last->element;
        DequeContainerT * tmp = dq->last;
        dq->last = dq->last->prev;
        if ( dq->last != NULL ) dq->last->next = NULL;
        if ( dq->head == tmp ) dq->head = dq->last;
        free ( tmp );
    }

    unlockDeque ( dq );
    return e;
}

void iterateDequeWith ( DequeT * dq, void ( * f ) ( ElementT * ) ) {
    lockDeque ( dq );

    DequeContainerT * c = dq->head;
    while ( c != NULL ) {
        f ( c->element );
        c = c->next;
    }

    unlockDeque ( dq );
}

void reverseIterateDequeWith ( DequeT * dq, void ( * f ) ( ElementT * ) ) {
    lockDeque ( dq );

    DequeContainerT * c = dq->last;
    while ( c != NULL ) {
        f ( c->element );
        c = c->prev;
    }

    unlockDeque ( dq );
}
