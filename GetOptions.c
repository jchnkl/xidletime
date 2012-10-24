#include "GetOptions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "Config.h"

static void usage ( const char * name ) {
    int i, noptions = 6;
    const char * options[] =
        { "[-i|--idlefile]"
        , "[-b|--base]"
        , "[-f|--idlefile]"
        , "[-t|--timeoutfile]"
        , "[-u|--busname]"
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
       {"idletime",      required_argument, 0, 'i' },
       {"base",          required_argument, 0, 'b' },
       {"idlefile",      required_argument, 0, 'f' },
       {"timeoutfile",   required_argument, 0, 't' },
       {"busname",       required_argument, 0, 'u' },
       {"objectpath",    required_argument, 0, 'o' },
       {"interfacename", required_argument, 0, 'n' },
       {0,               0,                 0, 0 }
   };

   memset ( options, 0, sizeof ( Options ) );

   while ( c != -1 ) {
       option_index = 0;
       c = getopt_long ( argc, argv, "i:b:f:t:u:o:n:", long_options, &option_index );

       if ( c == 0 ) { choice = option_index; } else { choice = c; }

       switch ( choice ) {
           case   0:
           case 'i': options->idletime      = strtol ( optarg, NULL, 10 ) * 1000; break;
           case   1:
           case 'b': options->base          = strtod ( optarg, NULL ); break;
           case   2:
           case 'f': options->idlefile      = optarg; break;
           case   3:
           case 't': options->timeoutfile   = optarg; break;
           case   4:
           case 'u': options->busName       = optarg; break;
           case   5:
           case 'o': options->objectPath    = optarg; break;
           case   6:
           case 'n': options->interfaceName = optarg; break;
           case '?':
           case ':': usage ( argv[0] );
           default: break;
       }
   }

   if ( options->idlefile == NULL || options->timeoutfile == NULL )
       usage ( argv[0] );

    options->busName =
        options->busName == NULL ? busName : options->busName;
    options->objectPath =
        options->objectPath == NULL ? objectPath : options->objectPath;
    options->interfaceName =
        options->interfaceName == NULL ? interfaceName : options->interfaceName;

}
