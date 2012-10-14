#ifndef __GETOPTIONS_H
#define __GETOPTIONS_H

typedef struct Options
    { unsigned int idletime
    ; const char * idlefile
    ; const char * timeoutfile
    ; const char * busName
    ; const char * objectPath
    ; const char * interfaceName
    ;
    } Options;

void getoptions ( Options * options, int argc, char ** argv );

#endif
