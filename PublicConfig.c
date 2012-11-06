#include "PublicConfig.h"

#include <stdlib.h>

PublicConfigT * makePublicConfig ( PublicConfigT * pc ) {
    if ( pc == NULL ) {
        pc = calloc ( 1, sizeof ( PublicConfigT ) );
        pc->dynamic = 1;
    } else {
        pc->dynamic = 0;
    }
    pthread_mutex_init ( &(pc->publiclock), NULL );
    return pc;
}

void destroyPublicConfig ( PublicConfigT * pc ) {
    pthread_mutex_destroy ( &(pc->publiclock) );
    if ( pc->dynamic == 1 ) { free ( pc ); }
}

void withPublicConfig ( PublicConfigT * pc, void (* f) (PublicConfigT *) ) {
    pthread_mutex_lock   ( &(pc->publiclock) );
    f ( pc );
    pthread_mutex_unlock ( &(pc->publiclock) );
}
