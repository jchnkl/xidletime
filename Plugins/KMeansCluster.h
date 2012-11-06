#ifndef __KMEANSCLUSTER_H
#define __KMEANSCLUSTER_H

typedef enum CmpTypeT { MEAN=0, FILL } CmpTypeT;

typedef struct BucketT {
    unsigned int value;
    struct BucketT * next;
} BucketT;

typedef struct ClusterT {
    unsigned int mean;
    unsigned int fillcount;
    BucketT * bucket;
} ClusterT;

typedef struct GroupT {
    const char * seed;
    unsigned int changed;
    unsigned int size;
    CmpTypeT    cmp_type;
    ClusterT * cluster;
} GroupT;

typedef struct GroupsT
    { unsigned int   ngroups
    ; GroupT       * groups
    ;
    } GroupsT;

int makeGroup
    ( int (* init) (int, int)
    , GroupT         * group
    , unsigned   int    size
    , CmpTypeT          comp
    , const      char * seed
    );

int makeGroups
    ( int (* init) (int, int)
    , GroupsT         *  groups
    , unsigned   int  *  size
    , CmpTypeT        *  comp
    , const      char ** seed
    );

int seedGroup ( GroupT * group );

int finalizeGroup ( GroupT * group );

void dumpGroup ( GroupT * group );

int minDistance ( GroupT * group, unsigned int * value );

void addKMeanValue ( GroupT * group, int * idx, unsigned int * value );

int findValue ( GroupT * group, unsigned int * value );

int addValue ( GroupT * group, unsigned int * value );

void distributeMeans ( GroupT * group );

void updateMeans ( GroupT * group );

void updateGroup ( GroupT * group );

void printGroup ( GroupT * group );


#endif
