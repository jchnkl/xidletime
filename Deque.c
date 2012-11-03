#include "Deque.h"

#include <stdlib.h>

static void dequeLock ( DequeT * dq ) {
    pthread_mutex_lock ( &(dq->dequelock) );
}

static void dequeUnlock ( DequeT * dq ) {
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
    dequeLock ( dq );
    int res = dq->head == NULL;
    dequeUnlock ( dq );
    return res;
}

void pushHead ( DequeT * dq, ElementT * e, ElementT * (* make) ( void ) ) {
    ContainerT * c = (ContainerT *) calloc ( 1, sizeof ( ContainerT ) );
    if ( e == NULL && make != NULL ) e = make();
    c->element = e;

    dequeLock ( dq );

    if ( dq->head == NULL ) {
        dq->head = c; dq->last = c;
    } else {
        c->next = dq->head;
        dq->head->prev = c;
        dq->head = c;
    }

    dequeUnlock ( dq );
}

void pushLast ( DequeT * dq, ElementT * e, ElementT * (* make) ( void ) ) {
    ContainerT * c = (ContainerT *) calloc ( 1, sizeof ( ContainerT ) );
    if ( e == NULL && make != NULL ) e = make();
    c->element = e;

    dequeLock ( dq );

    if ( dq->last == NULL ) {
        dq->head = c; dq->last = c;
    } else {
        dq->last->next = c;
        c->prev = dq->last;
        dq->last = c;
    }

    dequeUnlock ( dq );
}

ElementT * popHead ( DequeT * dq ) {
    ElementT * e = NULL;
    dequeLock ( dq );

    if ( dq->head != NULL ) {
        e = dq->head->element;
        ContainerT * tmp = dq->head;
        dq->head = dq->head->next;
        if ( dq->head != NULL ) dq->head->prev = NULL;
        if ( dq->last == tmp ) dq->last = dq->head;
        free ( tmp );
    }

    dequeUnlock ( dq );
    return e;
}

ElementT * popLast ( DequeT * dq ) {
    ElementT * e = NULL;
    dequeLock ( dq );

    if ( dq->last != NULL ) {
        e = dq->last->element;
        ContainerT * tmp = dq->last;
        dq->last = dq->last->prev;
        if ( dq->last != NULL ) dq->last->next = NULL;
        if ( dq->head == tmp ) dq->head = dq->last;
        free ( tmp );
    }

    dequeUnlock ( dq );
    return e;
}

void iterateWith ( DequeT * dq, void ( * f ) ( ElementT * ) ) {
    dequeLock ( dq );

    ContainerT * c = dq->head;
    while ( c != NULL ) {
        f ( c->element );
        c = c->next;
    }

    dequeUnlock ( dq );
}

void reverseIterateWith ( DequeT * dq, void ( * f ) ( ElementT * ) ) {
    dequeLock ( dq );

    ContainerT * c = dq->last;
    while ( c != NULL ) {
        f ( c->element );
        c = c->prev;
    }

    dequeUnlock ( dq );
}
