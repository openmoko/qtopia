MODULES_DIR=/etc/usbnet_services

StartService()
{
    # start network-dependant modules
    for file in $MODULES_DIR/*.sh; do
        if [ "$file" != "$MODULES_DIR/*.sh" ]; then
            eval $file start
        fi
    done
}

StopService()
{
    # stop network-dependant modules
    for file in $MODULES_DIR/*.sh; do
        if [ "$file" != "$MODULES_DIR/*.sh" ]; then
            eval $file stop
        fi
    done
}

findlauncherpids()
{
    ps ax | grep '[u]sbnet_services\.sh daemon' | awk '{print $1}'
}

case $1 in
daemon)
    while :; do
        # Wait for an external interface (eth* or wlan*) to exist
        # and be brought up.
        while ! ifconfig | grep -E '^(eth|wlan)' >/dev/null; do
            sleep 5
        done

        StartService

        # Wait for all external interfaces (eth* or wlan*) to disappear
        while ifconfig | grep -E '^(eth|wlan)' >/dev/null; do
            sleep 5
        done

        StopService
    done
    ;;
start)
    # Don't ever show output from this process and don't let it get killed!
    nohup $0 daemon >/dev/null 2>&1 &
    ;;
stop)
    kill `findlauncherpids` >/dev/null 2>&1
    StopService >/dev/null 2>&1
    ;;
*)
    echo "usage: $0 { start | stop }"
    exit 2
    ;;
esac

exit 0

