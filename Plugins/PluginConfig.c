#include "PluginConfig.h"

#include "HelloWorld.h"

const EventSourceConfigT sourceConfig[] =
    { {  0, NULL, helloWorldSource, NULL }
    , { -1, NULL, NULL,             NULL }
    };

const EventSinkConfigT sinkConfig[] =
    { {  0, NULL, helloWorldSink }
    , { -1, NULL, NULL           }
    };

const WireTableConfigT wireTable[] =
    { {  0, 0, {  0 } }
    , { -1, 0, {}     }
    };
