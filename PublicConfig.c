#include "PublicConfig.h"

#include <stdlib.h>

PublicConfigT * makePublicConfig ( PublicConfigT * pc ) {
    if ( pc == NULL ) {
        pc = calloc ( 1, sizeof ( PublicConfigT ) );
        pc->dynamic = 1;
    } else {
        pc->dynamic = 0;
    }
    pthread_spin_init ( &(pc->publiclock), PTHREAD_PROCESS_SHARED );
    return pc;
}

void destroyPublicConfig ( PublicConfigT * pc ) {
    pthread_spin_destroy ( &(pc->publiclock) );
    if ( pc->dynamic == 1 ) { free ( pc ); }
}

void withPublicConfig ( PublicConfigT * pc, void (* f) (PublicConfigT *) ) {
    pthread_spin_lock   ( &(pc->publiclock) );
    f ( pc );
    pthread_spin_unlock ( &(pc->publiclock) );
}
