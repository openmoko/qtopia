#!/bin/sh

/etc/rc.d/usbnet_services.sh stop

# notify Qtopia that some network devices may have disappeared 
qcop send "QPE/NetworkState" "updateNetwork()"
