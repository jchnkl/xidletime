#include "kMeans.h"

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

int makeGroup ( group_t * group, unsigned int size ) {

    if ( group == NULL ) {
        group = (group_t *) calloc ( size, sizeof ( group_t ) );
    } else {
        memset ( group, 0, sizeof ( group_t ) );
    }

    group->size = size;

    cluster_t * cluster = (cluster_t *) calloc ( size, sizeof ( cluster_t ) );
    group->cluster = &cluster[0];

    if ( group->cluster != NULL ) {
        return 0;
    } else {
        return -1;
    }
}

int seedGroup ( group_t * group, const char * path ) {

    unsigned int   i;
    long           length;
    size_t         tsize  = sizeof ( unsigned int );
    FILE         * stream;
    unsigned int * values;

    if ( path != NULL ) {
        group->path = path;
#ifdef DEBUG
        fprintf ( stderr, "open: %s\n", path );
#endif
        stream = fopen ( path, "a+" );
    }

    if ( stream != NULL ) {
        rewind ( stream );
        fseek ( stream, 0, SEEK_END );
        length = ftell ( stream );
        rewind ( stream );

#ifdef DEBUG
        fprintf ( stderr, "length: %li ", length );
#endif

        values = (unsigned int *) malloc ( length );

        fread ( values, tsize, length, stream );

#ifdef DEBUG
        fprintf ( stderr, "values: " );
#endif
        for ( i = 0; i < length / sizeof ( unsigned int ); i++ ) {
#ifdef DEBUG
            fprintf ( stderr, "%i ", values[i] );
#endif
            addValue ( group, &values[i] );
        }
#ifdef DEBUG
        fprintf ( stderr, "done.\n" );
#endif

        free ( values );
        fclose ( stream );

    } else {
        return -1;
    }

#ifdef DEBUG
    fprintf ( stderr, "makeGroup finished.\n" );
#endif
    return 0;
}

int finalizeGroup ( group_t * group ) {
    int i;
    FILE * stream = NULL;

    if ( group->path != NULL ) {
#ifdef DEBUG
        fprintf ( stderr, "open: %s\n", group->path );
#endif
        // stream = fopen ( group->path, "w+" );
    }

    for ( i = 0; i < group->size; i++ ) {
        bucket_t * bucket = group->cluster[i].bucket;
        while ( bucket != NULL ) {
            if ( stream != NULL ) {
#ifdef DEBUG
                fprintf ( stderr, "%u ", bucket->value );
#endif
                // fwrite ( &(bucket->value), sizeof ( unsigned int ), 1, stream );
            }
            bucket_t * tmp = bucket->next;
            tmp = bucket->next;
            free ( bucket );
            bucket = tmp;
        }
    }
#ifdef DEBUG
    fprintf ( stderr, "\n" );
#endif

    if ( stream != NULL ) {
        // fclose ( stream );
    }

    free ( group->cluster );

    return 0;
}

void dumpGroup ( group_t * group, const char * groupFile ) {
    int i;
    FILE * stream = fopen ( group->path, "w+" );

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
#ifdef DEBUG
    fprintf ( stderr, "ADDVALUE: %u; IDX: %i\n", *value, *idx );
#endif
    bucket_t * tmp = (bucket_t *) calloc ( 1, sizeof ( bucket_t ) );
    tmp->value = *value;
    tmp->next = group->cluster[*idx].bucket;
    group->cluster[*idx].bucket = tmp;
    group->cluster[*idx].fillcount++;
#ifdef DEBUG
    fprintf ( stderr, "ADDVALUE finished\n" );
#endif
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
#ifdef DEBUG
    fprintf ( stderr, "minDistance\n" );
#endif
    int idx = minDistance ( group, value );
#ifdef DEBUG
    fprintf ( stderr, "addKMeanValue\n" );
#endif
    addKMeanValue ( group, &idx, value );
#ifdef DEBUG
    fprintf ( stderr, "updateGroup\n" );
#endif
    updateGroup ( group );
    /*
    if ( group->path != NULL ) {
        FILE * stream = fopen ( group->path, "a" );
        fwrite ( value, sizeof ( unsigned int ), 1, stream );
        fclose ( stream );
    }
    */
    return idx;
}

void distributeMeans ( group_t * group ) {
    int i = 0;
#ifdef DEBUG
    fprintf ( stderr, "==================== distributeMeans begin ==================== \n");
#endif
    for ( i = 0; i < group->size; i++ ) {

        if ( group->cluster[i].bucket == NULL ) continue;

        // copy of pointer
        bucket_t * prev   = NULL; // group->cluster[i].bucket;
        bucket_t * bucket = group->cluster[i].bucket;

        while ( bucket != NULL && bucket->value != 0 ) {
            int idx = minDistance ( group, &bucket->value );

            bucket_t * next = bucket->next;

            if ( idx != i ) {
#ifdef DEBUG
                fprintf ( stderr, "moving %u from %u to %u\n", bucket->value, i, idx );
#endif

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
#ifdef DEBUG
    if ( group->changed != 0 ) {
        printMeans ( group );
    }
    fprintf ( stderr, "==================== distributeMeans finish ==================== \n");
#endif
}

void updateMeans ( group_t * group ) {
#ifdef DEBUG
    fprintf ( stderr, "==================== updateMeans begin ==================== \n");
#endif
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
#ifdef DEBUG
    fprintf ( stderr, "==================== updateMeans finish ==================== \n");
#endif
}

void updateGroup ( group_t * group ) {
    do {
        group->changed = 0;
#ifdef DEBUG
    fprintf ( stderr, "updateMeans\n" );
#endif
        updateMeans ( group );
#ifdef DEBUG
    fprintf ( stderr, "distributeMeans\n" );
#endif
        distributeMeans ( group );
    } while ( group->changed != 0 );

#ifdef DEBUG
    fprintf ( stderr, "qsort\n" );
#endif
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

    for ( i = 0; i < group->size; i++ ) {
        // fprintf ( stderr, "address: %u\t", &group->cluster[i] );
        fprintf ( stderr, "group: %u\t", i );
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

        /*
        int sum = 0, count = 0;
        while ( bucket != NULL ) {
            fprintf ( stderr, "%u ", bucket->value );
            count++; sum+= bucket->value;
            bucket = bucket->next;
        }
        */

        /*
        for ( int j = 0; j < group->cluster[i].fillcount % BUCKETSIZE; j++ ) {
            fprintf ( stderr, "%u ", group->cluster[i].values[j] );
        }
        */
        // fprintf ( stderr, "\ncalculated mean is: %f\n\n", sum / (double)count );
    }
}
