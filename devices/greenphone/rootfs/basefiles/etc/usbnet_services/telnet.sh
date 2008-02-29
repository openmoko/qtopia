#!/bin/sh

case $1 in
start)
    ENV=/etc/profile /usr/sbin/telnetd
    ;;
stop)
    kill $( pidof telnetd ) 2>/dev/null
    ;;
esac

exit 0

