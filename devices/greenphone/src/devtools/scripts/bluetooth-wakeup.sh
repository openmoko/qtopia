#!/bin/sh

killall hciattach
rmmod omega_bt
insmod omega_bt
BDADDR=`/mnt/user/tools/tat bdaddr`
hciattach /dev/ttyS1 stlc2500 921600 noflow $BDADDR
