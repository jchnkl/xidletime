#ifndef __GROUP_H
#define __GROUP_H

#include "KMeansCluster.h"

typedef struct GroupData
    { int (* init) (int, int)
    ; int ngroups
    ; GroupT * group
    ; unsigned int * size
    ; CmpTypeT * comp
    ; const char ** seed
    ;
    } GroupData;

#endif
