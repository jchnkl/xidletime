#include "DBusInterface.h"

// #include "../Config.h"
#include "XIdleTimer.h"
// #include "SignalEmitter.h"
extern const char * busName;
extern const char * objectPath;
extern const char * interfaceName;

static DBusConfig staticDBusConfig;

static void initDBus ( PublicConfigT * pc ) {

    if ( pc->dbusconn != NULL ) return;

    DBusError        err;

    dbus_error_init ( &err ); // initialise the errors


    // connect to the bus
    staticDBusConfig.connection = dbus_bus_get ( DBUS_BUS_SESSION, &err );


    // request a name on the bus

    dbus_bus_request_name ( staticDBusConfig.connection
                          , pc->options->busName
                          , DBUS_NAME_FLAG_REPLACE_EXISTING
                          , &err
                          );

    staticDBusConfig.busName = pc->options->busName;
    staticDBusConfig.objectPath = pc->options->objectPath;
    staticDBusConfig.interfaceName = pc->options->interfaceName;


}

static int dbusEmitSignal ( void ) {

    DBusMessage * msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests

    // create a signal and check for errors
    msg = dbus_message_new_signal ( staticDBusConfig.objectPath
                                  , staticDBusConfig.interfaceName
                                  , staticDBusConfig.signalName
                                  );

    if ( msg == NULL ) return -1;

    // send the message and flush the connection
    if ( ! dbus_connection_send ( staticDBusConfig.connection
                                , msg
                                , &serial
                                )
       ) return -1;

    dbus_connection_flush ( staticDBusConfig.connection );

    dbus_message_unref ( msg );
    return 0;

}

void dbusSendSignalSink ( EventSinkT * snk, EventSourceT * src ) {
    if ( snk->private == NULL ) {
        withPublicConfig ( snk->public, initDBus );
        snk->private = (void *)&staticDBusConfig;
    }

    if ( src->id == 1 && Reset == ((XTimerCallbackT *)src->private)->status ) {
        staticDBusConfig.signalName = "Reset";
    } else {
        staticDBusConfig.signalName = "Idle";
    }
    dbusEmitSignal();
}
