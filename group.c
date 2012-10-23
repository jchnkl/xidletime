#include "group.h"

#include <string.h>

void initGroups ( GroupData * gd ) {
    int i;

    memset ( gd->group, 0, gd->ngroups * sizeof ( group_t ) );

    for ( i = 0; i < gd->ngroups; i++ ) {
        makeGroup ( gd->init, &(gd->group[i]), gd->size[i], gd->comp[i], gd->seed[i] );
    }
}
