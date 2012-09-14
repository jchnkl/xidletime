#ifndef __BACKOFFSTRATEGY_H
#define __BACKOFFSTRATEGY_H

typedef struct BackOffStrategy
    { unsigned int (* strategy) ( unsigned int idletime, int * idlecount )
    ;
    } BackOffStrategy;

#endif
