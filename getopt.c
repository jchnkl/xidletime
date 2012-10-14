#include "getopt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void usage ( const char * name ) {
    int i, noptions = 6;
    const char * options[] =
        { "[-i|--idlefile]"
        , "[-f|--idlefile]"
        , "[-t|--timeoutfile]"
        , "[-b|--busname]"
        , "[-o|--objectpath]"
        , "[-n|--interfacename]"
        };

    fprintf ( stdout, "Usage: %s", name );

    for ( i = 0; i < noptions; i++ ) {
        fprintf ( stdout, " %s", options[i] );
    }

    fprintf ( stdout, "\n" );

    exit ( EXIT_FAILURE );
}

void getoptions ( Options * options, int argc, char ** argv ) {

   int c = 0, choice, option_index;

   static struct option long_options[] = {
       {"idletime",      required_argument, 0, 0 },
       {"busname",       required_argument, 0, 0 },
       {"objectpath",    required_argument, 0, 0 },
       {"interfacename", required_argument, 0, 0 },
       {0,               0,                 0, 0 }
   };

   memset ( options, 0, sizeof ( Options ) );

   while ( c != -1 ) {
       option_index = 0;
       c = getopt_long ( argc, argv, "t:b:o:i:", long_options, &option_index );

       if ( c == 0 ) { choice = option_index; } else { choice = c; }

       // strdup is ok here: this will be only called once at program start
       switch ( choice ) {
           case   0:
           case 't': options->idletime      = strtol ( optarg, NULL, 10 ); break;
           case   1:
           case 'b': options->busName       = strdup ( optarg ); break;
           case   2:
           case 'o': options->objectPath    = strdup ( optarg ); break;
           case   3:
           case 'i': options->interfaceName = strdup ( optarg ); break;
           case '?': break;
           default: break;
       }
   }

}
