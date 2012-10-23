#include "DBusSignalEmitter.h"

#include "SignalEmitter.h"

int initDBus ( DBusConfig * dbusconfig ) {

    DBusError        err;
    DBusConnection * conn;

    dbus_error_init ( &err ); // initialise the errors

    if ( dbusconfig == NULL ) return -1;

    if ( dbusconfig->busName == NULL
      || dbusconfig->objectPath == NULL
      || dbusconfig->interfaceName == NULL
       ) return -1;

    // connect to the bus
    conn = dbus_bus_get ( DBUS_BUS_SESSION, &err );

    if ( dbus_error_is_set ( &err ) || conn == NULL ) return -1;

    // request a name on the bus
    if ( dbus_bus_request_name ( conn
                               , dbusconfig->busName
                               , DBUS_NAME_FLAG_REPLACE_EXISTING
                               , &err
                               )
         != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER
      || dbus_error_is_set ( &err )
       ) return -1;

    dbusconfig->connection = conn;

    return 0;

}

void finalizeDBus ( DBusConfig * dbusconfig ) {
    if ( dbusconfig != NULL && dbusconfig->connection != NULL ) {
        dbus_connection_unref ( dbusconfig->connection );
    }
}

int dbusEmitSignal ( DBusConfig * dbusconfig ) {

    DBusMessage * msg;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests

    // create a signal and check for errors
    msg = dbus_message_new_signal ( dbusconfig->objectPath
                                  , dbusconfig->interfaceName
                                  , dbusconfig->signalName
                                  );

    if ( msg == NULL ) return -1;

    // send the message and flush the connection
    if ( ! dbus_connection_send ( dbusconfig->connection
                                , msg
                                , &serial
                                )
       ) return -1;

    dbus_connection_flush ( dbusconfig->connection );

    dbus_message_unref ( msg );
    return 0;

}

static int _dbusEmitSignal ( SignalEmitter * se, char * name ) {
    ((DBusConfig *)(se->data))->signalName = name;
    dbusEmitSignal ( se->data );
    return 0;
}

int getSignalEmitter ( DBusConfig * dbusconfig, SignalEmitter * signalemitter ) {
    if ( dbusconfig == NULL ) return -1;

    signalemitter->data = dbusconfig;
    signalemitter->emitSignal = _dbusEmitSignal;

    return 0;
}
