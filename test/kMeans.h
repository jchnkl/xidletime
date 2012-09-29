#ifndef __KMEANS_H
#define __KMEANS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

typedef struct bucket_t {
    unsigned int value;
    struct bucket_t * next;
} bucket_t;

typedef struct kmeans_t {
    unsigned int mean;
    unsigned int fillcount;
    bucket_t * bucket;
} kmeans_t;

typedef struct cluster_t {
    unsigned int changed;
    unsigned int size;
    kmeans_t * kmeans;
} cluster_t;

int minDistance ( cluster_t * cluster, unsigned int * value );

void addKMeanValue ( cluster_t * cluster, int * idx, unsigned int * value );

int findValue ( cluster_t * cluster, unsigned int * value );

int addValue ( cluster_t * cluster, unsigned int * value );

void distributeMeans ( cluster_t * cluster );

void updateMeans ( cluster_t * cluster );

void updateCluster ( cluster_t * cluster );

void printMeans ( cluster_t * cluster );


#endif
