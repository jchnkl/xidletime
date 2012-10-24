#ifndef __CALLBACK_H
#define __CALLBACK_H

typedef void * CallbackDataT;

typedef void ( * CallbackFunctionT ) ( CallbackDataT * );

typedef struct CallbackT
    { CallbackDataT     data
    ; CallbackFunctionT run
    ;
    } CallbackT;

#endif
