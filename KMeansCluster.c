#include "KMeansCluster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>

typedef int (* CmpFunT) ( const void * a, const void * b);

static
int compMean ( const void *clusterl, const void *clusterr ) {
    return ((ClusterT *)clusterl)->mean > ((ClusterT *)clusterr)->mean;
}

static
int compFill ( const void *clusterl, const void *clusterr ) {
    return ((ClusterT *)clusterl)->fillcount > ((ClusterT *)clusterr)->fillcount;
}

CmpFunT cmp_fun[] =
    { compMean
    , compFill
    };

int makeGroup
    ( int (* init) (int, int)
    , GroupT         * group
    , unsigned   int    size
    , CmpTypeT          comp
    , const      char * seed
    ) {

    int k;

    if ( group == NULL ) group = (GroupT *) calloc ( size, sizeof ( GroupT ) );

    group->size = size;
    group->cmp_type = comp;

    ClusterT * cluster = (ClusterT *) calloc ( size, sizeof ( ClusterT ) );
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
    , GroupsT         *  groups
    , unsigned   int  *  size
    , CmpTypeT        *  comp
    , const      char ** seed
    ) {

    int i;

    if ( size == NULL || comp == NULL ) return -1;

    if ( groups->groups == NULL ) {
        groups->groups = (GroupT *) calloc ( groups->ngroups, sizeof (GroupT *) );
    } else {
        memset ( groups->groups, 0, groups->ngroups * sizeof ( GroupT * ) );
    }

    for ( i = 0; i < groups->ngroups; i++ ) {
        makeGroup ( init, &(groups->groups[i]), size[i], comp[i], seed[i] );
    }

    return 0;
}

int seedGroup ( GroupT * group ) {

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

int finalizeGroup ( GroupT * group ) {
    int i;

    for ( i = 0; i < group->size; i++ ) {
        BucketT * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            BucketT * tmp = bucket->next;
            tmp = bucket->next;
            free ( bucket );
            bucket = tmp;
        }
    }

    free ( group->cluster );

    return 0;
}

void dumpGroup ( GroupT * group, const char * groupFile ) {
    int i;
    FILE * stream = fopen ( group->seed, "w+" );

    for ( i = 0; i < group->size; i++ ) {
        BucketT * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            fwrite ( &(bucket->value), sizeof ( unsigned int ), 1, stream );
            bucket = bucket->next;
        }
    }

    fclose ( stream );
}

int minDistance ( GroupT * group, unsigned int * value ) {
    int i, idx = 0, gdist = 0x7fffffff;
    for ( i = 0; i < group->size; i++ ) {
        int ldist = abs ( * value - group->cluster[i].mean );
        if ( ldist < gdist ) { gdist = ldist; idx = i; }
    }
    return idx;
}

void addKMeanValue ( GroupT * group, int * idx, unsigned int * value ) {
    BucketT * tmp = (BucketT *) calloc ( 1, sizeof ( BucketT ) );
    tmp->value = *value;
    tmp->next = group->cluster[*idx].bucket;
    group->cluster[*idx].bucket = tmp;
    group->cluster[*idx].fillcount++;
}

int findValue ( GroupT * group, unsigned int * value ) {
    int i;
    for ( i = 0; i < group->size; i++ ) {
        BucketT * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            if ( bucket->value == *value ) return i;
            bucket = bucket->next;
        }
    }
    return -1;
}

int addValue ( GroupT * group, unsigned int * value ) {
    int idx = minDistance ( group, value );
    addKMeanValue ( group, &idx, value );
    updateGroup ( group );
    return idx;
}

void distributeMeans ( GroupT * group ) {
    int i = 0;
    for ( i = 0; i < group->size; i++ ) {

        if ( group->cluster[i].bucket == NULL ) continue;

        // copy of pointer
        BucketT * prev   = NULL;
        BucketT * bucket = group->cluster[i].bucket;

        while ( bucket != NULL && bucket->value != 0 ) {
            int idx = minDistance ( group, &bucket->value );

            BucketT * next = bucket->next;

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

void updateMeans ( GroupT * group ) {
    int i;
    unsigned int newmean = 0;
    BucketT * bucket;
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

void updateGroup ( GroupT * group ) {
    do {
        group->changed = 0;
        updateMeans ( group );
        distributeMeans ( group );
    } while ( group->changed != 0 );

    qsort ( group->cluster
          , group->size
          , sizeof ( ClusterT )
          , cmp_fun[group->cmp_type]
          );
}

void printGroup ( GroupT * group ) {
    int i;
    BucketT * bucket;

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
