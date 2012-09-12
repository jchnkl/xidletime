#include "DBusSignalEmitter.h"

#include "SignalEmitter.h"

int initDBus ( DBusConfig * dbusconfig ) {

    DBusError        err;
    DBusConnection * conn;
    int              ret = 0;

    dbus_error_init ( &err ); // initialise the errors

    if ( dbusconfig == NULL ) { ret = -1; goto exit; }
    if ( dbusconfig->busName == NULL
      || dbusconfig->objectPath == NULL
      || dbusconfig->interfaceName == NULL
       ) { ret = -1; goto exit; }

    // connect to the bus
    conn = dbus_bus_get ( DBUS_BUS_SESSION, &err );

    if ( dbus_error_is_set ( &err ) || conn == NULL ) { ret = -1; goto exit; }

    // request a name on the bus
    if ( dbus_bus_request_name ( conn
                               , dbusconfig->busName
                               , DBUS_NAME_FLAG_REPLACE_EXISTING
                               , &err
                               )
         != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER
      || dbus_error_is_set ( &err )
       ) { ret = -1; goto exit; }

    dbusconfig->connection = conn;

exit:
    dbus_error_free ( &err );
    return ret;

}

void finalizeDBus ( DBusConfig * dbusconfig ) {
    if ( dbusconfig != NULL && dbusconfig->connection != NULL ) {
        dbus_connection_unref ( dbusconfig->connection );
    }
}

int dbusEmitSignal ( DBusConfig * dbusconfig ) {

    DBusMessage * msg;
    int           ret = 0;
    dbus_uint32_t serial = 0; // unique number to associate replies with requests

    // create a signal and check for errors
    msg = dbus_message_new_signal ( dbusconfig->objectPath
                                  , dbusconfig->interfaceName
                                  , dbusconfig->signalName
                                  );

    if ( msg == NULL ) { ret = -1; goto exit; }

    // send the message and flush the connection
    if ( ! dbus_connection_send ( dbusconfig->connection
                                , msg
                                , &serial
                                )
       ) { ret = -1; goto exit; }

    dbus_connection_flush ( dbusconfig->connection );

exit:
    dbus_message_unref ( msg );
    return ret;

}

static int _dbusEmitSignal ( SignalEmitter * se, char * name, void * args ) {
    ((DBusConfig *)(se->config))->signalName = name;
    dbusEmitSignal ( se->config );
    return 0;
}

int getSignalEmitter ( DBusConfig * dbusconfig, SignalEmitter * signalemitter ) {

    int ret = 0;
    if ( dbusconfig == NULL ) { ret = -1; goto exit; }

    signalemitter->config = dbusconfig;
    signalemitter->emitSignal = _dbusEmitSignal;

exit:
    return ret;
}
