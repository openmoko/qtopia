#!/bin/sh

# This flash script prevents the gui software from starting.

# To use this script copy it to a sd card as trolltech_flash.sh

echo "Trolltech null startup hook"

QTOPIA_TOOLS=/mnt/user/tools

# start usb networking
$QTOPIA_TOOLS/usbnet.sh load
$QTOPIA_TOOLS/usbnet.sh up &
$QTOPIA_TOOLS/usbnet.sh start_services

exit 0

