#include "kMeans.h"

#define NVALUES 1000
#define CLUSTERSIZE 16

int main ( int argc, char ** argv ) {

    cluster_t cluster;
    memset ( &cluster, 0, sizeof ( cluster ) );
    kmeans_t kmeans[CLUSTERSIZE];
    memset ( &kmeans, 0, sizeof ( kmeans ) );

    cluster.size = CLUSTERSIZE;
    cluster.kmeans = &kmeans[0];

    cluster.kmeans[0].mean = 3 * NVALUES / 4;
    cluster.kmeans[1].mean = NVALUES / 2;
    cluster.kmeans[2].mean = NVALUES / 4;

    // cluster.kmeans[0].mean = 0;
    // cluster.kmeans[1].mean = myIdleTime / 2;

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
    printMeans ( &cluster );
}
