#ifndef __GROUP_H
#define __GROUP_H

#include "KMeansCluster.h"

typedef struct GroupData
    { int (* init) (int, int)
    ; int ngroups
    ; group_t * group
    ; int * size
    ; cmp_type_t * comp
    ; const char ** seed
    ;
    } GroupData;

void initGroups ( GroupData * gd );

#endif
