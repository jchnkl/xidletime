#include <stdio.h>
#include <stdlib.h>

#include "Deque.h"

#define MAX 10

void printInt ( ElementT * e ) {
    fprintf ( stderr, "%i ", *(int*)e );
}

int main ( int argc, char ** argv ) {
    int i = 0;
    DequeT * dq = makeDeque ( NULL );

    fprintf ( stderr, "Pushin'\n" );
    for ( i = 0; i < MAX; ++i ) {
        int * j = (int *) calloc ( 1, sizeof ( int ) );
        int * k = (int *) calloc ( 1, sizeof ( int ) );
        *j = i; *k = i * 10;
        pushHead ( dq, j, NULL );
        pushLast ( dq, k, NULL );
    }

    fprintf ( stderr, "Printin'\n" );
    iterateWith ( dq, printInt );
    fprintf ( stderr, "\n" );
    reverseIterateWith ( dq, printInt );
    fprintf ( stderr, "\n" );

    fprintf ( stderr, "Poppin'\n" );
    for ( i = 0; i < MAX; ++i ) {
        ElementT * e;
        fprintf ( stderr, "Poppin Head: " );
        e = popHead ( dq );
        fprintf ( stderr, "%i\n", *(int*)e );
        if ( e != NULL ) free ( e );
        fprintf ( stderr, "Poppin Last: " );
        e = popLast ( dq );
        fprintf ( stderr, "%i\n", *(int*)e );
        if ( e != NULL ) free ( e );
    }

    fprintf ( stderr, "Pushin'\n" );
    pthread_t threads[2];
    for ( i = 0; i < MAX; ++i ) {
        int j = 2;
        void * getInt() {
            int * x = (int *) calloc ( 1, sizeof ( int ) );
            // *x = i * j++ * 10;
            return x;
        }
        void * t0 ( void * a ) { pushHead ( dq, NULL, getInt ); return NULL; }
        void * t1 ( void * a ) { pushLast ( dq, NULL, getInt ); return NULL; }
        pthread_create ( &threads[0], NULL, t0, NULL );
        pthread_create ( &threads[1], NULL, t1, NULL );
    }

    fprintf ( stderr, "Printin'\n" );
    iterateWith ( dq, printInt );
    fprintf ( stderr, "\n" );
    reverseIterateWith ( dq, printInt );

    destroyDeque ( dq, free );

    return 0;
}
