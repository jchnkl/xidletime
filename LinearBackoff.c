#include "LinearBackoff.h"

unsigned int linearBackoff ( unsigned int * idletime, int * idlecount ) {
    return (*idletime) * (*idlecount);
}
