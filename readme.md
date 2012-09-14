# xidletime
## Sends a DBus signal when you're idle!
The X server exposes a counter called "IDLETIME" which increases while the user
is idle. Using an alarm applications can be notified if a certain has been
reached.
This small program makes use of this feature for sending a signal over
the DBus IPC mechanism when the user defined timeout has been reached or the
counter is reset due to user action.

## Dependencies
* `Xlib` (of course..)
* `Xext` (`/usr/include/X11/extensions/sync.h`)
* `DBus`

Check with

    pkg-config --cflags --libs dbus-1 x11 xext

and

    pkg-config --modversion dbus-1 x11 xext
    1.6.4
    1.5.0
    1.3.1

## Compile

Type `make` in the src directory. Should work out of the box.

## Usage

    xidletime [-t|--idletime <timeout>]
              [-b|--busName <busName>]
              [-o|--objectPath <objectPath>]
              [-i|--interfaceName <interfaceName>]

`idletime`: specifies the time (in microseconds) after which an `Idle` signal is
sent. After an `Idle` signal, any user action, like moving the mouse or hitting
a key, will immediately result in a `Reset` signal.

`busName`, `objectPath` and `interfaceName` set the appropriate DBus parameters.
This should only be changed by experienced users. Proper form is enforced by
libdbus and will lead to program termination if not used correctly.

#### Default Parameters:

    timeout:       2000
    busName:       org.xidletime
    objectPath:    /org/xidletime
    interfaceName: org.xidletime
