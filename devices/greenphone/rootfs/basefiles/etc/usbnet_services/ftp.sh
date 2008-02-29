#!/bin/sh

case $1 in
start)
    /usr/sbin/ftpd -S
    ;;
stop)
    killall ftpd
    ;;
esac

exit 0

