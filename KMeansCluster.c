#include "KMeansCluster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>

typedef int (* cmp_fun_t) ( const void * a, const void * b);

static
int compMean ( const void *clusterl, const void *clusterr ) {
    return ((cluster_t *)clusterl)->mean > ((cluster_t *)clusterr)->mean;
}

static
int compFill ( const void *clusterl, const void *clusterr ) {
    return ((cluster_t *)clusterl)->fillcount > ((cluster_t *)clusterr)->fillcount;
}

cmp_fun_t cmp_fun[] =
    { compMean
    , compFill
    };

int makeGroup
    ( int (* init) (int, int)
    , group_t         * group
    , unsigned   int    size
    , cmp_type_t        comp
    , const      char * seed
    ) {

    int k;

    if ( group == NULL ) group = (group_t *) calloc ( size, sizeof ( group_t ) );

    group->size = size;
    group->cmp_type = comp;

    cluster_t * cluster = (cluster_t *) calloc ( size, sizeof ( cluster_t ) );
    group->cluster = &cluster[0];

    for ( k = 0; k < size; k++ ) { group->cluster[k].mean = init ( k, size ); }

    if ( seed != NULL ) {
        group->seed = seed;
        seedGroup ( group );
    }

    return 0;
}

int makeGroups
    ( int (* init) (int, int)
    , group_t         ** groups
    , unsigned   int     ngroups
    , unsigned   int  *  size
    , cmp_type_t      *  comp
    , const      char ** seed
    ) {

    int i;

    if ( size == NULL || comp == NULL ) return -1;

    if ( groups == NULL ) {
        groups = (group_t **) calloc ( ngroups , sizeof ( group_t * ) );
    } else {
        memset ( groups, 0, ngroups * sizeof ( group_t * ) );
    }

    for ( i = 0; i < ngroups; i++ ) {
        makeGroup ( init, groups[i], size[i], comp[i], seed[i] );
    }

}

int seedGroup ( group_t * group ) {

    unsigned int   i;
    long           length;
    size_t         tsize  = sizeof ( unsigned int );
    FILE         * stream;
    unsigned int * values;

    if ( group->seed != NULL ) {
        stream = fopen ( group->seed, "a+" );
    } else {
        return -1;
    }

    if ( stream != NULL ) {
        rewind ( stream );
        fseek ( stream, 0, SEEK_END );
        length = ftell ( stream );
        rewind ( stream );

        values = (unsigned int *) malloc ( length );

        fread ( values, tsize, length, stream );

        for ( i = 0; i < length / sizeof ( unsigned int ); i++ ) {
            addValue ( group, &values[i] );
        }

        free ( values );
        fclose ( stream );

    } else {
        return -1;
    }

    return 0;
}

int finalizeGroup ( group_t * group ) {
    int i;
    FILE * stream = NULL;

    if ( group->seed != NULL ) {
        // stream = fopen ( group->seed, "w+" );
    }

    for ( i = 0; i < group->size; i++ ) {
        bucket_t * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            if ( stream != NULL ) {
                // fwrite ( &(bucket->value), sizeof ( unsigned int ), 1, stream );
            }
            bucket_t * tmp = bucket->next;
            tmp = bucket->next;
            free ( bucket );
            bucket = tmp;
        }
    }

    if ( stream != NULL ) {
        // fclose ( stream );
    }

    free ( group->cluster );

    return 0;
}

void dumpGroup ( group_t * group, const char * groupFile ) {
    int i;
    FILE * stream = fopen ( group->seed, "w+" );

    for ( i = 0; i < group->size; i++ ) {
        bucket_t * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            fwrite ( &(bucket->value), sizeof ( unsigned int ), 1, stream );
            bucket = bucket->next;
        }
    }

    fclose ( stream );
}

int minDistance ( group_t * group, unsigned int * value ) {
    int i, idx = 0, gdist = 0x7fffffff;
    for ( i = 0; i < group->size; i++ ) {
        int ldist = abs ( * value - group->cluster[i].mean );
        if ( ldist < gdist ) { gdist = ldist; idx = i; }
    }
    return idx;
}

void addKMeanValue ( group_t * group, int * idx, unsigned int * value ) {
    bucket_t * tmp = (bucket_t *) calloc ( 1, sizeof ( bucket_t ) );
    tmp->value = *value;
    tmp->next = group->cluster[*idx].bucket;
    group->cluster[*idx].bucket = tmp;
    group->cluster[*idx].fillcount++;
}

int findValue ( group_t * group, unsigned int * value ) {
    int i;
    for ( i = 0; i < group->size; i++ ) {
        bucket_t * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            if ( bucket->value == *value ) return i;
            bucket = bucket->next;
        }
    }
    return -1;
}

int addValue ( group_t * group, unsigned int * value ) {
    int idx = minDistance ( group, value );
    addKMeanValue ( group, &idx, value );
    updateGroup ( group );
    return idx;
}

void distributeMeans ( group_t * group ) {
    int i = 0;
    for ( i = 0; i < group->size; i++ ) {

        if ( group->cluster[i].bucket == NULL ) continue;

        // copy of pointer
        bucket_t * prev   = NULL;
        bucket_t * bucket = group->cluster[i].bucket;

        while ( bucket != NULL && bucket->value != 0 ) {
            int idx = minDistance ( group, &bucket->value );

            bucket_t * next = bucket->next;

            if ( idx != i ) {

                group->changed++;
                addKMeanValue ( group, &idx, &bucket->value );

                bucket->value = 0;
                group->cluster[i].fillcount--;

                if ( bucket == group->cluster[i].bucket ) {
                    group->cluster[i].bucket = bucket->next;
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

            if ( group->cluster[i].fillcount == 0 ) {
                group->cluster[i].bucket = NULL;
                bucket = NULL;
            }

            bucket = next;
        }
    }
}

void updateMeans ( group_t * group ) {
    int i;
    unsigned int newmean = 0;
    bucket_t * bucket;
    for ( i = 0; i < group->size; i++ ) {
        if ( group->cluster[i].fillcount == 0 ) continue;
        bucket = group->cluster[i].bucket;
        while ( bucket != NULL && bucket->value != 0 ) {
            newmean += bucket->value;
            bucket = bucket->next;
        }
        group->cluster[i].mean = newmean / ( group->cluster[i].fillcount );
        newmean = 0;
    }
}

void updateGroup ( group_t * group ) {
    do {
        group->changed = 0;
        updateMeans ( group );
        distributeMeans ( group );
    } while ( group->changed != 0 );

    qsort ( group->cluster
          , group->size
          , sizeof ( cluster_t )
          , cmp_fun[group->cmp_type]
          );
}

void printGroup ( group_t * group ) {
    int i;
    bucket_t * bucket;

    struct winsize ws;
    ioctl ( 0, TIOCGWINSZ, &ws );

    fprintf ( stderr, "group with size %u, sorted by %i\n", group->size, group->cmp_type );
    for ( i = 0; i < group->size; i++ ) {

        fprintf ( stderr, "cluster: %u\t", i );
        fprintf ( stderr, "mean: %u\t", group->cluster[i].mean );
        fprintf ( stderr, "fillcount: %u\n", group->cluster[i].fillcount );

        if ( group->cluster[i].fillcount == 0 ) continue;

        bucket = group->cluster[i].bucket;

        unsigned int max = 0;
        while ( bucket != NULL ) {
            if ( bucket->value > max ) max = bucket->value;
            bucket = bucket->next;
        }

        unsigned int maxlen = 0;
        while ( max > 0 ) {
            maxlen++; max /= 10;
        }
        maxlen += 1;

        int printed = 0;
        bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            if ( printed >= ws.ws_col - maxlen ) {
                fprintf ( stdout, "\n" );
                printed = 0;
            }
            printed += fprintf ( stdout, "%-*u ", maxlen, bucket->value );
            bucket = bucket->next;
        }
        fprintf ( stdout, "\n\n" );

    }
}
