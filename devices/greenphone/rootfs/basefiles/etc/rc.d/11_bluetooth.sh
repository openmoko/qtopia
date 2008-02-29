
case $1 in
start)
    hciattach /dev/ttyS1 stlc2500

    # Device address is not actually burned into the device ROM, must be set in software
    # The address is stored in the TAT table
    # If the address is not set accordingly, run the fixbdaddr tool in devtools
    BDADDR=`/mnt/user/tools/tat bdaddr`
    /usr/sbin/hciconfig hci0 up
    /usr/sbin/bdaddr -i hci0 $BDADDR
    /usr/sbin/hciconfig hci0 reset

    # Make sure hcid and sdpd are started afterwards
    hcid
    sdpd
    ;;
stop)
    killall hcid sdpd hciattach >/dev/null 2>&1
    ;;
*)
    echo "usage: $0 { start | stop }"
    exit 2
    ;;
esac

exit 0

