#!/bin/sh

HOME=$( awk -F : '/^root/ { print $6 }' < /etc/passwd )

rm -f $HOME/.ssh/authorized_keys

/etc/usbnet_services/sshkey.sh start

