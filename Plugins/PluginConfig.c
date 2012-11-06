#include "PluginConfig.h"

#include "HelloWorld.h"
#include "XIdleTimer.h"
#include "AdaptiveTimeout.h"
#include "DBusInterface.h"

const EventSourceConfigT sourceConfig[] =
    { {  1, NULL, xIdleTimerSource, NULL }
    , { -1, NULL, NULL,             NULL }
    };

const EventSinkConfigT sinkConfig[] =
    { {  1, NULL, xIdleTimerSink      }
    , {  2, NULL, adaptiveTimeoutSink }
    , {  3, NULL, dbusSendSignalSink  }
    , { -1, NULL, NULL                }
    };

const WireTableConfigT wireTable[] =
    { {  3, 1, { 1, 2, 3 } }
    , { -1, 0, {      } }
    };
