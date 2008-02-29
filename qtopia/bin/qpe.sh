#!/bin/sh

# Sample start-up script for Qtopia.
#
# Instead of using a script like this, you may instead set
# the defaults for these variables in the file:
#
#    src/libraries/qtopia/custom-<platform>.h
#
# for your platform.

cmd=$0
speed=""
while [ -n "$1" ]
do
    case "$1" in
     --ir)
	export QTOPIA_PHONE_DEVICE="/dev/ircomm"
    ;; --sim)
	export QTOPIA_PHONE_DEVICE="sim:localhost"
	export QTOPIA_PHONE_DEVICE_LOW="sim:localhost"
    ;; --gsm0710)
	export QTOPIA_PHONE_DEVICE="/dev/ttyU0"
	export QTOPIA_PHONE_DEVICE_LOW="/dev/ttyU1"
    ;; --speed)
	shift
	export speed=$1
    ;; --modem)
	export QTOPIA_PHONE_DEVICE="/dev/modem"
    esac
    shift
done

if [ -n "$speed" ]
then
    export QTOPIA_PHONE_DEVICE="$QTOPIA_PHONE_DEVICE:$speed"
fi

$QPEDIR/bin/qpe
