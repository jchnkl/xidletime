#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dbus/dbus.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

unsigned int idletime      = 2000;
const char * busName       = "org.IdleTime";
const char * objectPath    = "/org/IdleTime";
const char * interfaceName = "org.IdleTime";

long XSyncValueToLong ( XSyncValue *value ) {
    return ( (long) XSyncValueHigh32 ( *value ) << 32
           | XSyncValueLow32 ( *value )
           );
}

DBusConnection * dbusInit ( int * );

int dbusSendSignal ( DBusConnection *, char * );

int main ( int argc, char ** argv ) {

    int ret = 0, major = 0, minor = 0;
    int ev_base = 0, err_base = 0;

    DBusConnection * conn = NULL;
    conn = dbusInit ( &ret );
    if ( ret == -1 || conn == NULL ) { exit ( EXIT_FAILURE ); }

    Display *dpy = XOpenDisplay ( "" );
    Window root = DefaultRootWindow ( dpy );

    ret = XSelectInput ( dpy, root, XSyncAlarmNotifyMask );
    if ( ret == 0 ) { exit ( EXIT_FAILURE ); }

    ret = XSyncInitialize ( dpy, &major, &minor );
    if ( ret == 0 ) { exit ( EXIT_FAILURE ); }
    printf ( "major: %i, minor: %i\n", major, minor );

    XSyncQueryExtension ( dpy, &ev_base, &err_base );
    printf ( "ev_base: %i, err_base: %i\n", ev_base, err_base );

    int i = 0, nxssc = 0;
    XSyncSystemCounter * xssc = XSyncListSystemCounters ( dpy, &nxssc );

    XSyncSystemCounter * idlecounter = NULL;
    for (i = 0; i < nxssc; i++) {
        if ( 0 == strcmp ( xssc[i].name, "IDLETIME" ) ) {
            printf ( "%s\n", xssc[i].name );
            idlecounter = &xssc[i];
        }
    }

    if ( idlecounter != NULL ) {

        unsigned long flags = XSyncCACounter
                            | XSyncCAValueType
                            | XSyncCATestType
                            | XSyncCAValue
                            | XSyncCADelta
                            ;

        XEvent xev;
        XSyncValue delta, timeout;

        XSyncAlarmAttributes xsaa;

        XSyncIntToValue (&delta, 0);
        XSyncIntToValue (&timeout, idletime);

        xsaa.trigger.counter    = idlecounter->counter;
        xsaa.trigger.value_type = XSyncAbsolute;
        xsaa.trigger.test_type  = XSyncPositiveComparison;
        xsaa.trigger.wait_value = timeout;
        xsaa.delta              = delta;

        XSyncAlarm setAlarm ( Bool reset, XSyncValue to ) {
            if ( reset ) {
                xsaa.trigger.test_type  = XSyncPositiveComparison;
                xsaa.trigger.wait_value = to;
            } else {
                xsaa.trigger.test_type  = XSyncNegativeComparison;
                xsaa.trigger.wait_value = to;
            }
            return XSyncCreateAlarm ( dpy, flags, &xsaa );
        }

        void changeAlarm ( XSyncAlarm * alarm, Bool reset, XSyncValue to ) {
            if ( reset ) {
                xsaa.trigger.test_type  = XSyncPositiveComparison;
                xsaa.trigger.wait_value = to;
            } else {
                xsaa.trigger.test_type  = XSyncNegativeComparison;
                xsaa.trigger.wait_value = to;
            }
            XSyncChangeAlarm ( dpy, *alarm, flags, &xsaa );
        }

        Time last;
        XSyncAlarm alarm = setAlarm ( True, timeout );
        while ( 1 ) {
            XNextEvent ( dpy, &xev );

            if ( xev.type == ev_base + XSyncAlarmNotify ) {
                XSyncAlarmNotifyEvent * xane = (XSyncAlarmNotifyEvent *) &xev;

                if ( last != xane->time ) {
                    fprintf ( stderr, "last!=xane->time\n" );
                }

                Bool lessThan = XSyncValueLessThan ( xane->counter_value
                                                   , xane->alarm_value
                                                   );

                if ( lessThan && last != xane->time ) {
                    fprintf ( stderr, "XSyncValueLessThan TRUE\n" );
                    xsaa.trigger.test_type = XSyncPositiveComparison;
                    dbusSendSignal ( conn, "Reset" );
                } else if ( last != xane->time ) {
                    fprintf ( stderr, "XSyncValueLessThan FALSE\n" );
                    xsaa.trigger.test_type = XSyncNegativeComparison;
                    dbusSendSignal ( conn, "Idle" );
                }

                XSyncChangeAlarm ( dpy, alarm, flags, &xsaa );
                last = xane->time;

                fprintf ( stderr
                        , "alarm; time: %i, serial: %lu, counter_value: %lu, alarm_value: %lu\n"
                        , (int)xane->time
                        , xane->serial
                        , XSyncValueToLong ( &(xane->counter_value) )
                        , XSyncValueToLong ( &(xane->alarm_value) )
                        );

            }

        }
    }

    XSyncFreeSystemCounterList ( xssc );

    XCloseDisplay ( dpy );

    dbus_connection_close ( conn );

    return 0;
}


DBusConnection * dbusInit ( int * error ) {

    int              ret;
    DBusError        err;
    DBusConnection * conn;

    dbus_error_init ( &err ); // initialise the errors

    // connect to the bus
    conn = dbus_bus_get ( DBUS_BUS_SESSION, &err );

    if ( dbus_error_is_set ( &err ) || conn == NULL ) {
#ifdef DEBUG
        fprintf ( stderr, "Connection Error (%s)\n", err.message );
#endif
        dbus_error_free ( &err ) ;
        *error = -1;
        return NULL;
    }
    /*
    if  ( NULL == conn) {
        exit ( 1);
    }
    */

    // request a name on the bus
    ret = dbus_bus_request_name ( conn
                                , busName
                                , DBUS_NAME_FLAG_REPLACE_EXISTING
                                , &err
                                );

    if ( dbus_error_is_set ( &err )
      || DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret ) {
#ifdef DEBUG
        fprintf ( stderr, "Name Error (%s)\n", err.message );
#endif
        dbus_error_free ( &err );
        *error = -1;
        return NULL;
    }
    /*
    if ( DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret ) {
        exit ( 1 );
    }
    */

    return conn;

}

int dbusSendSignal ( DBusConnection * conn, char * signalName ) {

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;

    // create a signal and check for errors
    msg = dbus_message_new_signal ( objectPath
                                  , interfaceName
                                  , signalName
                                  );

    if ( NULL == msg ) {
#ifdef DEBUG
        fprintf ( stderr, "Message Null\n" );
#endif
        return -1;
    }

    /*
    // append arguments onto signal
    if ( data != NULL ) {
        DBusMessageIter args;
        dbus_message_iter_init_append ( msg, &args );
        if ( ! dbus_message_iter_append_basic ( &args, DBUS_TYPE_UINT32, &data ) ) {
#ifdef DEBUG
            fprintf ( stderr, "Out Of Memory!\n" );
#endif
            return -1;
        }
    }
    */

    // send the message and flush the connection
    if  ( ! dbus_connection_send ( conn, msg, &serial ) ) {
#ifdef DEBUG
        fprintf ( stderr, "Out Of Memory!\n" );
#endif
        return -1;
    }

    dbus_connection_flush ( conn );

    // free the message
    dbus_message_unref ( msg );

    return 0;

}
