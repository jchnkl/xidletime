#include "BackoffStrategy.h"
#include "NoBackoff.h"
#include "LinearBackoff.h"

Strategies strategies[] =
    { { NONE,   noBackoff     }
    , { LINEAR, linearBackoff }
    };
