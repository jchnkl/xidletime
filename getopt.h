#ifndef __GETOPT_H
#define __GETOPT_H

typedef struct Options
    { unsigned int idletime
    ; const char * busName
    ; const char * objectPath
    ; const char * interfaceName
    ;
    } Options;

void getoptions ( Options * options, int argc, char ** argv );

#endif
