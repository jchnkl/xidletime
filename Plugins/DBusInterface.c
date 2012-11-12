#include "DBusInterface.h"

#include <sys/select.h>

#include "XIdleTimer.h"

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

    pc->dbusconn = dbus.connection;
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

void * dbusReceiveSource ( EventSourceT * src ) {
    if ( src->private == NULL ) {
        withPublicConfig ( src->public, initDBus );
        src->private = (void *)&dbus;
    }

    int socketfd;
    fd_set readSet;
    dbus_connection_get_socket ( src->public->dbusconn, &socketfd );
    FD_ZERO ( &readSet );
    FD_SET ( socketfd, &readSet );

    // loop, testing for new messages
    while ( 1 ) {
        DBusMessage * msg = NULL;

        select ( socketfd+1, &readSet, NULL, NULL, NULL );

        dbus_connection_read_write_dispatch ( dbus.connection, 0 );
        msg = dbus_connection_pop_message ( dbus.connection );

        // loop again if we haven't got a message
        if ( msg == NULL ) continue;

        // check this is a method call for the right interface and method
        if ( dbus_message_is_method_call ( msg
                                         , dbus.busName
                                         , "ScreenSaverSuspend"
                                         )
           ) {
            src->private = (void *) dbus_message_copy ( msg );

            dbus_message_set_serial ( (DBusMessage *)(src->private)
                                    , dbus_message_get_serial ( msg )
                                    );

            src->eq->queueEvent ( src->eq, src );
        }

        // free the message
        dbus_message_unref(msg);
    }

    return NULL;
}

void dbusEmitSignalSink ( EventSinkT * snk, EventSourceT * src ) {
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
