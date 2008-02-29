#!/bin/sh

/etc/rc.d/usbnet_services.sh start

# notify Qtopia that some new network devices may have appeared 
qcop send "QPE/NetworkState" "updateNetwork()"

