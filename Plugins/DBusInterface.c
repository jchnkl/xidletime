#include "DBusInterface.h"

// #include "../Config.h"
#include "XIdleTimer.h"
// #include "SignalEmitter.h"
extern const char * busName;
extern const char * objectPath;
extern const char * interfaceName;

static DBusT dbus;

static void initDBus ( PublicConfigT * pc ) {

    if ( pc->dbusconn != NULL ) return;

    // for thread safe dbus api
    dbus_threads_init_default();

    // connect to the bus
    dbus.connection = dbus_bus_get ( DBUS_BUS_SESSION, NULL );

    dbus_bus_request_name ( dbus.connection
                          , pc->options->busName
                          , DBUS_NAME_FLAG_REPLACE_EXISTING
                          , NULL
                          );

    dbus.busName = pc->options->busName;
    dbus.objectPath = pc->options->objectPath;
    dbus.interfaceName = pc->options->interfaceName;
}

static void dbusEmitSignal ( void ) {

    DBusMessage * msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests

    // create a signal and check for errors
    msg = dbus_message_new_signal ( dbus.objectPath
                                  , dbus.interfaceName
                                  , dbus.signalName
                                  );

    if ( msg == NULL ) return;

    // send the message and flush the connection
    dbus_connection_send ( dbus.connection
                         , msg
                         , &serial
                         );

    dbus_connection_flush ( dbus.connection );

    dbus_message_unref ( msg );

}

void dbusSendSignalSink ( EventSinkT * snk, EventSourceT * src ) {
    if ( snk->private == NULL ) {
        withPublicConfig ( snk->public, initDBus );
        snk->private = (void *)&dbus;
    }

    if ( src->id == 1 && Reset == ((XTimerT *)src->private)->status ) {
        dbus.signalName = "Reset";
    } else {
        dbus.signalName = "Idle";
    }
    dbusEmitSignal();
}
