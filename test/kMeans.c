#include "kMeans.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int makeCluster ( cluster_t * cluster, unsigned int size, const char * path ) {

    long           length;
    size_t         tsize  = sizeof ( unsigned int );
    FILE         * stream;
    unsigned int * values;

    memset ( cluster, 0, sizeof ( cluster_t ) );
    cluster->size = size;

    kmeans_t * kmeans = (kmeans_t *) calloc ( size, sizeof ( kmeans_t ) );
    cluster->kmeans = &kmeans[0];

    if ( path != NULL ) {
        // cluster->path = strdup ( path );
        cluster->path = path;
        fprintf ( stderr, "open: %s\n", path );
        stream = fopen ( path, "a+" );
    }

    if ( stream != NULL ) {
        rewind ( stream );
        fseek ( stream, 0, SEEK_END );
        length = ftell ( stream );
        rewind ( stream );

        fprintf ( stderr, "length: %li ", length );

        values = (unsigned int *) malloc ( length );

        fread ( values, tsize, length, stream );

        fprintf ( stderr, "values: " );
        for ( unsigned int i = 0; i < length / sizeof ( unsigned int ); i++ ) {
            fprintf ( stderr, "%i ", values[i] );
            addValue ( cluster, &values[i] );
        }
        fprintf ( stderr, "done.\n" );

        free ( values );
        fclose ( stream );

    } else {
        return -1;
    }

    fprintf ( stderr, "makeCluster finished.\n" );
    return 0;
}

int finalizeCluster ( cluster_t * cluster ) {
    FILE * stream = NULL;

    if ( cluster->path != NULL ) {
        fprintf ( stderr, "open: %s\n", cluster->path );
        stream = fopen ( cluster->path, "w+" );
    }

    for ( int i = 0; i < cluster->size; i++ ) {
        bucket_t * bucket = cluster->kmeans[i].bucket;
        while ( bucket != NULL ) {
            if ( stream != NULL ) {
                fprintf ( stderr, "%u ", bucket->value );
                fwrite ( &(bucket->value), sizeof ( unsigned int ), 1, stream );
            }
            bucket_t * tmp = bucket->next;
            tmp = bucket->next;
            free ( bucket );
            bucket = tmp;
        }
    }
    fprintf ( stderr, "\n" );

    if ( stream != NULL ) {
        fclose ( stream );
    }

    free ( cluster->kmeans );

    return 0;
}

int minDistance ( cluster_t * cluster, unsigned int * value ) {
    int idx = 0, gdist = 0x7fffffff;
    for ( int i = 0; i < cluster->size; i++ ) {
        int ldist = abs ( * value - cluster->kmeans[i].mean );
        if ( ldist < gdist ) { gdist = ldist; idx = i; }
    }
    return idx;
}

void addKMeanValue ( cluster_t * cluster, int * idx, unsigned int * value ) {
#ifdef DEBUG
    fprintf ( stderr, "ADDVALUE: %u; IDX: %i\n", *value, *idx );
#endif
    bucket_t * tmp = (bucket_t *) calloc ( 1, sizeof ( bucket_t ) );
    tmp->value = *value;
    tmp->next = cluster->kmeans[*idx].bucket;
    cluster->kmeans[*idx].bucket = tmp;
    cluster->kmeans[*idx].fillcount++;
}

int findValue ( cluster_t * cluster, unsigned int * value ) {
    for ( int i = 0; i < cluster->size; i++ ) {
        bucket_t * bucket = cluster->kmeans[i].bucket;
        while ( bucket != NULL ) {
            if ( bucket->value == *value ) return i;
            bucket = bucket->next;
        }
    }
    return -1;
}

int addValue ( cluster_t * cluster, unsigned int * value ) {
    /* int idx = findValue ( cluster, value );
    if ( idx == -1 ) {
    */
        int idx = minDistance ( cluster, value );
        addKMeanValue ( cluster, &idx, value );
        updateCluster ( cluster );
        return idx;
    /*
    } else {
        return -1;
    }
    */
}

void distributeMeans ( cluster_t * cluster ) {
#ifdef DEBUG
    fprintf ( stderr, "==================== distributeMeans begin ==================== \n");
#endif
    for ( int i = 0; i < cluster->size; i++ ) {

        // bucket_t * prev, * next; //   = &cluster->kmeans[i].bucket;
        // bucket_t * tmp; //   = cluster->kmeans[i].bucket;

        if ( cluster->kmeans[i].bucket == NULL ) continue;

        // copy of pointer
        bucket_t * prev   = NULL; // cluster->kmeans[i].bucket;
        bucket_t * bucket = cluster->kmeans[i].bucket;

        while ( bucket != NULL && bucket->value != 0 ) {
            int idx = minDistance ( cluster, &bucket->value );

            bucket_t * next = bucket->next;

            if ( idx != i ) {
#ifdef DEBUG
                fprintf ( stderr, "moving %u from %u to %u\n", bucket->value, i, idx );
#endif

                cluster->changed++;
                addKMeanValue ( cluster, &idx, &bucket->value );

                /*
                if ( prev == NULL ) {
                    cluster->kmeans[i].bucket = bucket->next;
                } else if ( bucket->next == NULL ) {
                    prev->next = NULL;
                } else {
                    prev->next = bucket->next;
                }
                */
                // cluster->kmeans[i].bucket->value = 0;
                bucket->value = 0;
                cluster->kmeans[i].fillcount--;
                // free ( bucket );

                if ( bucket == cluster->kmeans[i].bucket ) {
                    cluster->kmeans[i].bucket = bucket->next;
                    free ( bucket );
                } else {
                    // prev is old bucket ( prev == bucket )
                    // old_bucket->next = old_bucket->next->next
                    prev->next = bucket->next;
                    free ( bucket );
                }

            } else {
                prev = bucket;
            }

            if ( cluster->kmeans[i].fillcount == 0 ) {
                cluster->kmeans[i].bucket = NULL;
                bucket = NULL;
            }

            bucket = next;
        }

        /*
        for ( int j = 0; j < cluster->kmeans[i].fillcount % BUCKETSIZE; j++ ) {
            unsigned int idx = minDistance ( cluster, &cluster->kmeans[i].values[j] );
            if ( idx != i ) {
                cluster->changed++;
                addKMeanValue ( cluster, &idx, &cluster->kmeans[i].values[j] );
                cluster->kmeans[i].values[j] = 0;
                cluster->kmeans[i].fillcount--;
            }
        }
        */
    }
#ifdef DEBUG
    if ( cluster->changed != 0 ) {
        printMeans ( cluster );
    }
    fprintf ( stderr, "==================== distributeMeans finish ==================== \n");
#endif
}

void updateMeans ( cluster_t * cluster ) {
#ifdef DEBUG
    fprintf ( stderr, "==================== updateMeans begin ==================== \n");
#endif
    unsigned int newmean = 0;
    bucket_t * bucket;
    for ( int i = 0; i < cluster->size; i++ ) {
        if ( cluster->kmeans[i].fillcount == 0 ) continue;
        // if ( cluster->kmeans[i].bucket == NULL ) continue;
        // fprintf ( stderr, "calculating kmeans[%i] for\t", i );
        bucket = cluster->kmeans[i].bucket;
        while ( bucket != NULL && bucket->value != 0 ) {
            // fprintf ( stderr, "%u ", bucket->value );
            // if ( bucket->value == 0 ) exit ( EXIT_FAILURE );

            newmean += bucket->value;
            bucket = bucket->next;
        }
        cluster->kmeans[i].mean = newmean / ( cluster->kmeans[i].fillcount );
        /*
        fprintf ( stderr, "\nnewmean: %u / %u = %u\n"
                , newmean
                , cluster->kmeans[i].fillcount
                , cluster->kmeans[i].mean
                );
        */
        /*
        for ( int j = 0; j < cluster->kmeans[i].fillcount; j++ ) {
            newmean += cluster->kmeans[i].values[j];
        }
        cluster->kmeans[i].mean = newmean / ( cluster->kmeans[i].fillcount % BUCKETSIZE );
        */
        newmean = 0;
    }
#ifdef DEBUG
    fprintf ( stderr, "==================== updateMeans finish ==================== \n");
#endif
}

void updateCluster ( cluster_t * cluster ) {
    do {
        cluster->changed = 0;
        updateMeans ( cluster );
        distributeMeans ( cluster );
    } while ( cluster->changed != 0 );

    int compMeans ( const void *kmeansl, const void *kmeansr ) {
        return ((kmeans_t *)kmeansl)->mean > ((kmeans_t *)kmeansr)->mean;
    }

    qsort ( cluster->kmeans, cluster->size, sizeof ( kmeans_t ), compMeans );
}

void printMeans ( cluster_t * cluster ) {
    bucket_t * bucket;

    for ( int i = 0; i < cluster->size; i++ ) {
        // fprintf ( stderr, "address: %u\t", &cluster->kmeans[i] );
        fprintf ( stderr, "cluster: %u\t", i );
        fprintf ( stderr, "mean: %u\t", cluster->kmeans[i].mean );
        fprintf ( stderr, "fillcount: %u\n", cluster->kmeans[i].fillcount );

        bucket = cluster->kmeans[i].bucket;

        while ( bucket != NULL ) {
            fprintf ( stderr, "%u ", bucket->value );
            bucket = bucket->next;
        }
        /*
        for ( int j = 0; j < cluster->kmeans[i].fillcount % BUCKETSIZE; j++ ) {
            fprintf ( stderr, "%u ", cluster->kmeans[i].values[j] );
        }
        */
        fprintf ( stderr, "\n" );
    }
}
