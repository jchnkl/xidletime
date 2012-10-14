#ifndef __KMEANS_H
#define __KMEANS_H

typedef enum cmp_type_t { MEAN=0, FILL } cmp_type_t;

typedef struct bucket_t {
    unsigned int value;
    struct bucket_t * next;
} bucket_t;

typedef struct cluster_t {
    unsigned int mean;
    unsigned int fillcount;
    bucket_t * bucket;
} cluster_t;

typedef struct group_t {
    const char * path;
    unsigned int changed;
    unsigned int size;
    cmp_type_t cmp_type;
    cluster_t * cluster;
} group_t;

int makeGroup ( group_t * group, unsigned int size );

int seedGroup ( group_t * group, const char * path );

int finalizeGroup ( group_t * group );

void dumpGroup ( group_t * group, const char * groupFile );

int minDistance ( group_t * group, unsigned int * value );

void addKMeanValue ( group_t * group, int * idx, unsigned int * value );

int findValue ( group_t * group, unsigned int * value );

int addValue ( group_t * group, unsigned int * value );

void distributeMeans ( group_t * group );

void updateMeans ( group_t * group );

void updateGroup ( group_t * group );

void printGroup ( group_t * group );


#endif