#ifndef __BACKOFFSTRATEGY_H
#define __BACKOFFSTRATEGY_H

enum StrategyType { NONE, LINEAR };

typedef unsigned int StrategyType ;

struct Strategy
    { StrategyType type
    ; unsigned int (* backoff) ( unsigned int * idletime, int * idlecount )
    ;
    };

typedef struct Strategies { struct Strategy strategy; } Strategies;

#endif
