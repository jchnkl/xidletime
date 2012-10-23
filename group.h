#ifndef __GROUP_H
#define __GROUP_H

#include "KMeansCluster.h"

typedef struct GroupData
    { int (* init) (int, int)
    ; int ngroups
    ; GroupT * group
    ; int * size
    ; CmpTypeT * comp
    ; const char ** seed
    ;
    } GroupData;

void initGroups ( GroupData * gd );

#endif
