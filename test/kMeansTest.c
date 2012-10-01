#include "stdio.h"

#include "kMeans.h"

#define NVALUES 10
#define CLUSTERSIZE 8

int main ( int argc, char ** argv ) {

    cluster_t cluster;
    makeCluster ( &cluster, CLUSTERSIZE, "/tmp/clustertest" );

    cluster.kmeans[0].mean = 3 * NVALUES / 4;
    cluster.kmeans[1].mean = NVALUES / 2;
    cluster.kmeans[2].mean = NVALUES / 4;

    // cluster.kmeans[0].mean = 0;
    // cluster.kmeans[1].mean = myIdleTime / 2;

    for ( unsigned int i = 0; i < NVALUES; i++ ) {
        addValue ( &cluster, &i );
    }

    if ( 0 ) {
    for ( unsigned int i = NVALUES / 4; i < NVALUES / 2; i++ ) {
        unsigned int val = i; // * ( i % CLUSTERSIZE );
        if ( val != 0 ) {
            addValue ( &cluster, &val );
            /*
            if ( i % 10 == 0 ) {
                printMeans ( &cluster );
            }
            */
        }
    }

    for ( unsigned int i = NVALUES / 2; i < 3 * NVALUES / 4; i++ ) {
        unsigned int val = i; // * ( i % CLUSTERSIZE );
        if ( val != 0 ) {
            addValue ( &cluster, &val );
            /*
            if ( i % 10 == 0 ) {
                printMeans ( &cluster );
            }
            */
        }
    }

    for ( unsigned int i = 1; i < NVALUES / 4; i++ ) {
        unsigned int val = i; // * ( i % CLUSTERSIZE );
        if ( val != 0 ) {
            addValue ( &cluster, &val );
            /*
            if ( i % 10 == 0 ) {
                printMeans ( &cluster );
            }
            */
        }
    }

    for ( unsigned int i = 3 * NVALUES / 4; i < NVALUES; i++ ) {
        unsigned int val = i; // * ( i % CLUSTERSIZE );
        if ( val != 0 ) {
            addValue ( &cluster, &val );
            /*
            if ( i % 10 == 0 ) {
                printMeans ( &cluster );
            }
            */
        }
    }


    int count = 0, zeros = 0;
    for ( unsigned int i = 0; i < CLUSTERSIZE; i++ ) {
        bucket_t * bucket = cluster.kmeans[i].bucket;
        while ( bucket != NULL ) {
            if ( bucket->value != 0 ) {
                count++;
            } else {
                zeros++;
            }
            bucket = bucket->next;
        }
    }

    fprintf ( stderr, "count: %i; zeros: %i\n", count, zeros );
    }

    printMeans ( &cluster );
    finalizeCluster ( &cluster );
}
