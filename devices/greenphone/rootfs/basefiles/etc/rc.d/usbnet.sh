#!/bin/sh

findlauncherpids()
{
    ps ax | grep '[u]sbnet_services\.sh daemon' | awk '{print $1}'
}

case $1 in
daemon)
    # wait for network device to become available
    until ifconfig eth0 2>/dev/null >/dev/null; do
        sleep 5
    done

    # default settings
    MODE=static
    IPADDR=10.10.10.20
    NETMASK=255.255.255.255
    BROADCAST=255.255.255.255
    GATEWAY=10.10.10.21

    # use users network settings if they exists
    homes=`ls -1 /mnt/disk2/home | sort -nr`
    for i in $homes; do
        if [ -r /mnt/disk2/home/$i/Settings/Network/eth0 ]; then
            . /mnt/disk2/home/$i/Settings/Network/eth0
            break
        fi
    done

    # bring up network interface
    case $MODE in
        dhcp)
            # use a custom script which does does not always set up
            # the default route
            /sbin/udhcpc -s /usr/share/udhcpc/usbnet.script -i eth0
            ;;
        static)
            /sbin/ifconfig eth0 up
            /sbin/ifconfig eth0 $IPADDR netmask $NETMASK broadcast $BROADCAST

            # add host route to the gateway
            /sbin/route add $GATEWAY dev eth0

            # add a net route for the subnet if NETMASK != 255.255.255.255
            if [ "$NETMASK" != "255.255.255.255" ]; then
                eval $(/bin/ipcalc -s -n $IPADDR $NETMASK)
                /sbin/route add $NETWORK dev eth0
            fi

            # only update the default route if no default route is defined
            if ! route -n | grep '^0\.0\.0\.0 *[^ ]* *0\.0\.0\.0 *UG' >/dev/null; then
                /sbin/route add default gw $GATEWAY
            fi
            ;;
        *)
            ;;
    esac
    ;;
start)
    # Don't ever show output from this process and don't let it get killed!
    nohup $0 daemon >/dev/null 2>&1 &
    ;;
stop)
    kill `findlauncherpids` >/dev/null 2>&1
    ;;
*)
    echo "usage: $0 { start | stop }"
    exit 2
    ;;
esac
    
exit 0

