#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "XConfig.h"
#include "IdleMonitor.h"
#include "SignalEmitter.h"
#include "DBusSignalEmitter.h"

unsigned int idletime      = 2000;
const char * busName       = "org.xidletime";
const char * objectPath    = "/org/xidletime";
const char * interfaceName = "org.xidletime";

typedef struct
    { unsigned int idletime
    ; const char * busName
    ; const char * objectPath
    ; const char * interfaceName
    ;
    } Args;

void getargs ( Args * args, int argc, char ** argv );

int main ( int argc, char ** argv ) {

    int ret = 0;

    Args args = { idletime      = idletime
                , busName       = busName
                , objectPath    = objectPath
                , interfaceName = interfaceName
                };

    getargs ( &args, argc, argv );

    XConfig xconfig;
    memset ( &xconfig, 0, sizeof ( XConfig ) );
    SignalEmitter signalemitter;
    DBusConfig dbusconfig = { busName = args.busName
                            , objectPath = args.objectPath
                            , interfaceName = args.interfaceName
                            };
    IdleMonitorConfig idlemonitorconfig;
    memset ( &idlemonitorconfig, 0, sizeof ( IdleMonitorConfig ) );
    idlemonitorconfig.idletime = args.idletime;

    if ( -1 == initDBus ( &dbusconfig ) ) { goto exit; }

    if ( -1 == getSignalEmitter ( &dbusconfig, &signalemitter ) ) { goto exit; }

    if ( -1 == initXConfig ( &xconfig ) ) { goto exit; }

    if ( -1 == initIdleMonitor ( &xconfig, &idlemonitorconfig ) ) { goto exit; }

    if ( -1 == runIdleMonitor ( &xconfig, &idlemonitorconfig, &signalemitter ) ) {
        goto exit;
    }

exit:
    finalizeXConfig ( &xconfig );
    finalizeIdleMonitor ( &idlemonitorconfig );
    finalizeDBus ( &dbusconfig );
    return ret;

}

void getargs ( Args * args, int argc, char ** argv ) {

   int c = 0, choice, option_index;

   static struct option long_options[] = {
       {"idletime",      required_argument, 0, 0 },
       {"busname",       required_argument, 0, 0 },
       {"objectpath",    required_argument, 0, 0 },
       {"interfacename", required_argument, 0, 0 },
       {0,               0,                 0, 0 }
   };

   while ( c != -1 ) {
       option_index = 0;
       c = getopt_long ( argc, argv, "t:b:o:i:", long_options, &option_index );

       if ( c == 0 ) { choice = option_index; } else { choice = c; }

       // strdup is ok here: this will be only called once at program start
       switch ( choice ) {
           case   0:
           case 't': args->idletime      = strtol ( optarg, NULL, 10 ); break;
           case   1:
           case 'b': args->busName       = strdup ( optarg ); break;
           case   2:
           case 'o': args->objectPath    = strdup ( optarg ); break;
           case   3:
           case 'i': args->interfaceName = strdup ( optarg ); break;
           case '?': break;
           default: break;
       }
   }

}
