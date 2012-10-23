#!/bin/bash

typeset -i DIMMING

DIMPID=
DIMMING=0
BLSTATE=$(xbacklight -get)

coproc {
    dbus-monitor --profile --session "type='signal',sender='org.xidletime'"
}

trap "pkill -P ${COPROC_PID}" EXIT

while read -u ${COPROC[0]}; do
    case $REPLY in
        *Idle*)
            echo "Idle"
            BLSTATE=$(xbacklight -get)
            DIMMING=1
            xbacklight -time 800 -steps 20 -set 0 &
            DIMPID=$!
            ;;
        *Reset*)
            echo "Reset"
            if [ ${DIMMING} -eq 1 -a -n "${DIMPID}" ]; then
                kill ${DIMPID} 2>&1 >/dev/null
            fi
            xbacklight -time 0 -set ${BLSTATE}
            DIMMING=0
            ;;
    esac
done
