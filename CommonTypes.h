#ifndef __COMMONTYPES_H
#define __COMMONTYPES_H

typedef struct EventQueueT  EventQueueT;
typedef struct EventSourceT EventSourceT;
typedef struct EventSinkT   EventSinkT;

typedef struct EventSourceConfigT EventSourceConfigT;
typedef struct EventSinkConfigT EventSinkConfigT;
typedef struct WireTableConfigT WireTableConfigT;

typedef int IdentT;
typedef void * PublicConfig;
typedef void * PrivateConfig;
typedef void * (* eventRunner) ( EventSourceT * );
typedef void * (* eventCallback) ( void );

#endif
