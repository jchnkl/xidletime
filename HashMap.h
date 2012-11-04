#ifndef __CONNECTOR_H
#define __CONNECTOR_H

#include <sys/types.h>

typedef unsigned int KeyT;

typedef unsigned int HashValueT;

typedef void ElementT;

typedef struct ContainerT
    { KeyT key
    ; ElementT * element
    ;
    } ContainerT;

typedef struct HashMapT HashMapT;

typedef HashValueT (* HashFunT) ( HashMapT *, KeyT );

struct HashMapT
    { size_t size
    ; double resize
    ; unsigned int dynamic
    ; pthread_mutex_t hashlock
    ; HashFunT hashfun
    ; ContainerT * container
    ;
    };

HashMapT * makeHashMap ( HashMapT *, size_t, HashFunT );

void destroyHashMap ( HashMapT *, void ( * ) (ElementT *) );

void insert ( HashMapT *, KeyT, ElementT * );

ElementT * lookup ( HashMapT *, KeyT );

ElementT * delete ( HashMapT *, KeyT, void ( * ) (ElementT *) );

void iterateWith ( HashMapT *, void ( * ) ( ElementT * ) );

void reverseIterateWith ( HashMapT *, void ( * ) ( ElementT * ) );

#endif
