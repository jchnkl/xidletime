#include "HashMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static KeyT naiveHashFun ( HashMapT * hm, ElementT * e ) {
    return ( (unsigned long)e % hm->size );
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
    ContainerT * oldcontainer = hm->container;

    hm->size = hm->size + hm->size * hm->resize;
    hm->container = calloc ( hm->size, sizeof ( ContainerT ) );

    KeyT key;
    for ( i = 0; i < oldsize; ++i ) {
        if ( oldcontainer[i].element != NULL ) {
            key = hm->hashfun ( hm, oldcontainer[i].element );
            hm->container[key].key = key;
            hm->container[key].element = oldcontainer[i].element;
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
    hm->container = (ContainerT *) calloc ( size, sizeof ( ContainerT ) );

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

KeyT insert ( HashMapT * hm, ElementT * e ) {
    lockHashMap ( hm );
    KeyT key = hm->hashfun ( hm, e );

    if ( hm->container[key].element == NULL ) {
        hm->container[key].element = e;
        unlockHashMap ( hm );
    } else {
        unlockHashMap ( hm );
        resize ( hm );
        key = insert ( hm, e );
    }

    hm->container[key].key = key;

    return key;
}

ElementT * lookup ( HashMapT * hm, KeyT key ) {
    ElementT * e;
    lockHashMap ( hm );

    if ( hm->size < key ) {
        e = NULL;
    } else {
        e = hm->container[key].element;
    }

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

void iterateWith ( HashMapT * hm, void ( * f ) ( ElementT * ) ) {
    int i;
    lockHashMap ( hm );

    for ( i = 0; i < hm->size; ++i ) {
        if ( hm->container[i].element != NULL ) {
            f ( hm->container[i].element );
        }
    }

    unlockHashMap ( hm );
}

void reverseIterateWith ( HashMapT * hm, void ( * f ) ( ElementT * ) ) {
    int i;
    lockHashMap ( hm );

    for ( i = hm->size - 1; i >= 0; --i ) {
        if ( hm->container[i].element != NULL ) {
            f ( hm->container[i].element );
        }
    }

    unlockHashMap ( hm );
}
