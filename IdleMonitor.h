#ifndef __IDLEMONITOR_H
#define __IDLEMONITOR_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "XConfig.h"
#include "SignalEmitter.h"

typedef struct
    { unsigned int           idletime
    ; int                    ev_base
    ; int                    err_base
    ; XSyncSystemCounter   * counter
    ; XSyncAlarmAttributes * attributes
    ;
    } IdleMonitorConfig;

int initIdleMonitor ( XConfig * xconfig, IdleMonitorConfig * imc );

void finalizeIdleMonitor ( IdleMonitorConfig * imc );

int runIdleMonitor ( XConfig           * xconfig
                   , IdleMonitorConfig * imc
                   , SignalEmitter     * se
                   );

int checkIdleMonitorConfig ( IdleMonitorConfig * imc );

#endif
