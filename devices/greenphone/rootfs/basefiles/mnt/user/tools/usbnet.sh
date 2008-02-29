#!/bin/sh

. /etc/profile

case $1 in
    load)
        # load usb modules
        insmod usbdcore
        insmod net_fd.o if_name=eth
        insmod bvd_bi
        ;;
    unload)
        rmmod bvd_bi
        rmmod net_fd
        ;;
    up)
        # wait for network device to become available
        until ifconfig eth0 2>/dev/null >/dev/null; do
            sleep 1
        done

        # default settings
        MODE=static
        IPADDR=10.10.10.20
        NETMASK=255.255.255.255
        BROADCAST=255.255.255.255
        GATEWAY=10.10.10.20

        # if it exists use the users network settings
        if [ -r $HOME/Settings/Network/eth0 ]; then
            . $HOME/Settings/Network/eth0
        else
            homes=`ls -1 /mnt/disk2/home | sort -nr`
            for i in $homes; do
                if [ -r /mnt/disk2/home/$i/Settings/Network/eth0 ]; then
                    . /mnt/disk2/home/$i/Settings/Network/eth0
                    break
                fi
            done
        fi

        # bring up network interface
        case $MODE in
            dhcp)
                /sbin/udhcpc -i eth0
                ;;
            static)
                /sbin/ifconfig eth0 $IPADDR up
                /sbin/ifconfig eth0 netmask $NETMASK broadcast $BROADCAST
                /sbin/route del default
                /sbin/route add default gw $GATEWAY
                ;;
            *)
                ;;
        esac
        ;;
    down)
        /sbin/ifconfig eth0 down
        ;;
    *)
        echo "$0 {load|unload|up|down}"
        exit 1;
        ;;
esac
