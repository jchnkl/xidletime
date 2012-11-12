#include "HashMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static HashValueT naiveHashFun ( HashMapT * hm, KeyT key ) {
    return key % hm->size;
}

static void lockHashMap ( HashMapT * hm ) {
    pthread_mutex_lock ( &(hm->hashlock) );
}

static void unlockHashMap ( HashMapT * hm ) {
    pthread_mutex_unlock ( &(hm->hashlock) );
}

static void resize ( HashMapT * hm ) {
    int i;
    lockHashMap ( hm );

    size_t oldsize = hm->size;
    HashMapContainerT * oldcontainer = hm->container;

    hm->size = hm->size + hm->size * hm->resize;
    hm->container = calloc ( hm->size, sizeof ( HashMapContainerT ) );

    HashValueT hashvalue;
    for ( i = 0; i < oldsize; ++i ) {
        if ( oldcontainer[i].element != NULL ) {
            hashvalue = hm->hashfun ( hm, oldcontainer[i].key );
            hm->container[hashvalue].key = oldcontainer[i].key;
            hm->container[hashvalue].element = oldcontainer[i].element;
        }
    }

    free ( oldcontainer );
    unlockHashMap ( hm );
}

HashMapT * makeHashMap ( HashMapT * hm, size_t size, HashFunT hashfun ) {
    if ( hm == NULL ) {
        hm = (HashMapT *) calloc ( 1, sizeof ( HashMapT ) );
        hm->dynamic = 1;
    } else {
        hm->dynamic = 0;
    }

    if ( hashfun == NULL ) {
        hm->hashfun = naiveHashFun;
    } else {
        hm->hashfun = hashfun;
    }

    hm->size = size;
    hm->resize = 0.5;

    pthread_mutex_init ( &(hm->hashlock), NULL );
    hm->container =
        (HashMapContainerT *) calloc ( size, sizeof ( HashMapContainerT ) );

    return hm;
}

void destroyHashMap ( HashMapT * hm, void (* destroy) (ElementT *) ) {
    int i;
    if ( destroy != NULL ) {
        for ( i = 0; i < hm->size; i++ ) {
            destroy ( hm->container[i].element );
        }
    }
    pthread_mutex_destroy ( &(hm->hashlock) );
    free ( hm->container );
}

void insert ( HashMapT * hm, KeyT key, ElementT * e ) {
    lockHashMap ( hm );
    HashValueT hashvalue = hm->hashfun ( hm, key );

    if ( hm->container[hashvalue].element == NULL ) {
        hm->container[hashvalue].element = e;
        unlockHashMap ( hm );
    } else {
        unlockHashMap ( hm );
        resize ( hm );
        insert ( hm, key, e );
    }

    hm->container[hashvalue].key = key;
}

ElementT * lookup ( HashMapT * hm, KeyT key ) {
    ElementT * e;
    lockHashMap ( hm );

    e = hm->container[ hm->hashfun ( hm, key ) ].element;

    unlockHashMap ( hm );

    return e;
}

ElementT * delete ( HashMapT * hm, KeyT key, void (* destroy) (ElementT *) ) {
    ElementT * e;
    lockHashMap ( hm );

    fprintf ( stderr, "delete got key %u, with %i\n", key, *(int*)hm->container[key].element );
    if ( key < hm->size && hm->container[key].element != NULL ) {
        fprintf ( stderr, "rm'ing key %u, with %i\n", key, *(int*)hm->container[key].element );
        e = hm->container[key].element;
        hm->container[key].key = 0;
        if ( destroy != NULL ) destroy ( hm->container[key].element );
        hm->container[key].element = NULL;
    } else {
        e = NULL;
    }

    unlockHashMap ( hm );
    return e;
}

void iterateHashMapWith ( HashMapT * hm, void ( * f ) ( ElementT * ) ) {
    int i;
    lockHashMap ( hm );

    for ( i = 0; i < hm->size; ++i ) {
        if ( hm->container[i].element != NULL ) {
            f ( hm->container[i].element );
        }
    }

    unlockHashMap ( hm );
}

void reverseIterateHashMapWith ( HashMapT * hm, void ( * f ) ( ElementT * ) ) {
    int i;
    lockHashMap ( hm );

    for ( i = hm->size - 1; i >= 0; --i ) {
        if ( hm->container[i].element != NULL ) {
            f ( hm->container[i].element );
        }
    }

    unlockHashMap ( hm );
}
