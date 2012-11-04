#include <stdio.h>
#include <stdlib.h>

#include "HashMap.h"

int main ( int argc, char ** argv ) {
    int i = 0;

    KeyT myHash ( HashMapT * hm, ElementT * e ) { return *(int *)e; }

    HashMapT * hm = makeHashMap ( NULL, 10, NULL /* , myHash */ );

    KeyT key;
    for ( i = 0; i < 20; ++i ) {
        int * j = (int *) malloc ( sizeof ( int ) );
        *j = i;
        if ( i == 10 ) {
            key = insert ( hm, j );
            fprintf ( stderr
                    , "inserting %i, got key: %u\n"
                    , i
                    , key
                    );
        } else {
            fprintf ( stderr
                    , "inserting %i, got key: %u\n"
                    , i
                    , insert ( hm, j )
                    );
        }
    }

    // delete ( hm, 7, free );
    delete ( hm, key, free );

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
