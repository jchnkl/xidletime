#ifndef __GROUP_H
#define __GROUP_H

#include "kmeans.h"

typedef struct groupData
    { int (* init) (int, int)
    ; int ngroups
    ; group_t * group
    ; int * size
    ; cmp_type_t * comp
    ; const char ** seed
    ;
    } groupData;

void initGroups ( groupData * gd );

#endif
