# xidletime
## Sends a DBus signal when you're idle!
The X server exposes a counter called "IDLETIME" which increases while the user
is idle. Using an alarm applications can be notified if a certain has been
reached.

This small program makes use of this feature for sending a signal over
the DBus IPC mechanism when the user defined timeout has been reached or the
counter is reset due to user action.

A heuristic takes care of {in,de}creasing the timeout. If the user repeatedly
hits a key or moves the mouse in very short time frame after the Idle state was
entered the timeout is raised.

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

    xidletime [-i|--idletime <timeout>]
              [-b|--base <base>]
              [-f|--idlefile <idlefile>]
              [-t|--timeoutfile <timeoutfile>]
              [-u|--busName <busName>]
              [-o|--objectPath <objectPath>]
              [-n|--interfaceName <interfaceName>]

`idletime`: specifies the time (in microseconds) after which an `Idle` signal is
sent. After an `Idle` signal, any user action, like moving the mouse or hitting
a key, will immediately result in a `Reset` signal.

`base` specifies how the function for calculates the timeout should behave.
It indicates the point where the function flips from values greater than 1 to
values smaller than 1, which will decrease the timeout.

To get an idea of how this parameter influences the function draw this graph
using gnuplot:

    plot [0:99] (-1.0 * log(100/base) / log(base)) + log(x) / log(base)

`idlefile` and `timeoutfile` are files where statistical data is kept. Deleting
them will do no harm, but the adaption process has to begin anew.

`busName`, `objectPath` and `interfaceName` set the appropriate DBus parameters.
This should only be changed by experienced users. Proper form is enforced by
libdbus and will lead to program termination if not used correctly.

#### Default Parameters:

    busName:       org.xidletime
    objectPath:    /org/xidletime
    interfaceName: org.xidletime

## Example script
In `examples` a bash script can be found which shows how xidletime can be used
together with `dbus-monitor` and `xbacklight` to make the monitor dim when the
computer is idle.
