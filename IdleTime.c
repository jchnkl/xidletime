#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "XConfig.h"
#include "IdleMonitor.h"
#include "SignalEmitter.h"
#include "DBusSignalEmitter.h"

unsigned int timeout       = 2000;
const char * busName       = "org.IdleTime";
const char * objectPath    = "/org/IdleTime";
const char * interfaceName = "org.IdleTime";


int main ( int argc, char ** argv ) {

    int ret = 0;

    XConfig xconfig;
    memset ( &xconfig, 0, sizeof ( XConfig ) );
    SignalEmitter signalemitter;
    DBusConfig dbusconfig = { busName = busName
                            , objectPath = objectPath
                            , interfaceName = interfaceName
                            };
    IdleMonitorConfig idlemonitorconfig;
    memset ( &idlemonitorconfig, 0, sizeof ( IdleMonitorConfig ) );
    idlemonitorconfig.timeout = timeout;

    if ( -1 == initDBus ( &dbusconfig ) ) { goto exit; }

    if ( -1 == getSignalEmitter ( &dbusconfig, &signalemitter ) ) { goto exit; }

    if ( -1 == initXConfig ( &xconfig ) ) { goto exit; }

    if ( -1 == initIdleMonitor ( &xconfig, &idlemonitorconfig ) ) { goto exit; }

    if ( -1 == runIdleMonitor ( &xconfig, &idlemonitorconfig, &signalemitter ) ) {
        goto exit;
    }

exit:
    finalizeXConfig ( &xconfig );
    finalizeIdleMonitor ( &idlemonitorconfig );
    finalizeDBus ( &dbusconfig );
    return ret;

}
