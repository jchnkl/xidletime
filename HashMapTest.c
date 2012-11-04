#include <stdio.h>
#include <stdlib.h>

#include "HashMap.h"

int main ( int argc, char ** argv ) {
    int i = 0;

    KeyT myHash ( HashMapT * hm, ElementT * e ) { return *(int *)e; }

    HashMapT * hm = makeHashMap ( NULL, 10, NULL /* , myHash */ );

    for ( i = 0; i < 20; ++i ) {
        int * j = (int *) malloc ( sizeof ( int ) );
        *j = i;
        insert ( hm, i, j );
        fprintf ( stderr , "inserting %i with key: %i\n", i, *j );
    }

    delete ( hm, 3, free );
    delete ( hm, 7, free );
    delete ( hm, 13, free );
    delete ( hm, 17, free );

    void printInt ( ElementT * e ) { fprintf ( stderr, "%i ", *(int *)e ); }
    fprintf ( stderr, "iterating forward\n" );
    iterateWith ( hm, printInt );
    fprintf ( stderr, "\n" );

    fprintf ( stderr, "iterating backward\n" );
    reverseIterateWith ( hm, printInt );
    fprintf ( stderr, "\n" );

    destroyHashMap ( hm, free );

    return 0;
}
