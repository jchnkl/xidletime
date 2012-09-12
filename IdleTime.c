#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "XConfig.h"

unsigned int idletime      = 2000;
const char * busName       = "org.IdleTime";
const char * objectPath    = "/org/IdleTime";
const char * interfaceName = "org.IdleTime";


int main ( int argc, char ** argv ) {

    int ret = 0;

    XConfig xconfig;
    memset ( &xconfig, 0, sizeof ( XConfig ) );



    if ( -1 == initXConfig ( &xconfig ) ) { goto exit; }


    }

    finalizeXConfig ( &xconfig );
    return 0;

}
