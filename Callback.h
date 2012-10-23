#ifndef __CALLBACK_H
#define __CALLBACK_H

typedef struct CallbackT
    { void * data
    ; void ( * run ) ( struct CallbackT * )
    ;
    } CallbackT;

#endif
