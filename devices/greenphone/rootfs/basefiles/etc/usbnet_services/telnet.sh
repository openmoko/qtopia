#!/bin/sh

case $1 in
start)
    ENV=/etc/profile /usr/sbin/telnetd
    ;;
stop)
    killall telnetd
    ;;
esac

exit 0

