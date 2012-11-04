#ifndef __HASHMAP_H
#define __HASHMAP_H

#include <sys/types.h>

typedef unsigned int KeyT;

typedef unsigned int HashValueT;

typedef void ElementT;

typedef struct HashMapContainerT
    { KeyT key
    ; ElementT * element
    ;
    } HashMapContainerT;

typedef struct HashMapT HashMapT;

typedef HashValueT (* HashFunT) ( HashMapT *, KeyT );

struct HashMapT
    { size_t size
    ; double resize
    ; unsigned int dynamic
    ; pthread_mutex_t hashlock
    ; HashFunT hashfun
    ; HashMapContainerT * container
    ;
    };

HashMapT * makeHashMap ( HashMapT *, size_t, HashFunT );

void destroyHashMap ( HashMapT *, void ( * ) (ElementT *) );

void insert ( HashMapT *, KeyT, ElementT * );

ElementT * lookup ( HashMapT *, KeyT );

ElementT * delete ( HashMapT *, KeyT, void ( * ) (ElementT *) );

void iterateHashMapWith ( HashMapT *, void ( * ) ( ElementT * ) );

void reverseIterateHashMapWith ( HashMapT *, void ( * ) ( ElementT * ) );

#endif
