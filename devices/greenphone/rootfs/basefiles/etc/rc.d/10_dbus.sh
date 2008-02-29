
case $1 in
start)
    dbus-daemon --system
    ;;
stop)
    killall dbus-daemon
    ;;
*)
    echo "usage: $0 { start | stop }"
    exit 2
    ;;
esac

exit 0

