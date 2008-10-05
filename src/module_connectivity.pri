#This file contains projects that make up the Connectivity module.

PROJECTS*=\
    settings/netsetup \
    plugins/network/lan \           #LAN/WLAN support
    plugins/network/dialing \       #Dialup/GPRS support


PROJECTS*=\
    server/infrastructure/signalstrength \      #QSignalSource backend
    server/net/netserver \                      #base net server


enable_vpn:PROJECTS*=server/net/vpn             #VPN backend

PROJECTS*=\
    server/comm/usbgadget \             #usb gadget task
    server/ui/usbgadgetselector         #usb gadget selection dialog
