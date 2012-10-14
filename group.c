#include "group.h"

#include <string.h>

void initGroups ( GroupData * gd ) {
    int i, k;

    memset ( gd->group, 0, gd->ngroups * sizeof ( group_t ) );

    for ( i = 0; i < gd->ngroups; i++ ) {
        gd->group[i].cmp_type = gd->comp[i];
        makeGroup ( &(gd->group[i]), gd->size[i] );

        for ( k = 0; k < gd->size[i]; k++ ) {
            gd->group[i].cluster[k].mean = gd->init ( k, gd->size[i] );
        }

        seedGroup ( &(gd->group[i]), gd->seed[i] );
    }
}
