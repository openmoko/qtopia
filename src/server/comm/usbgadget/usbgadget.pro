!qbuild{
qtopia_project(server_lib)
TARGET=qpe_comm_usbgadget

depends(server/core_server)
depends(server/processctrl/taskmanagerentry)
depends(libraries/qtopiacomm/usb)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(usbgadget.pri)

usbgadget.files=$$device_overrides(/etc/default/Trolltech/Usb.conf)
!isEmpty(usbgadget.files) {
    usbgadget.path=/etc/default/Trolltech
    INSTALLS+=usbgadget
}

usbethernetservice.files=$$QTOPIA_DEPOT_PATH/services/UsbGadget/Ethernet/qpe
usbethernetservice.path=/services/UsbGadget/Ethernet
INSTALLS+=usbethernetservice

usbstorageservice.files=$$QTOPIA_DEPOT_PATH/services/UsbGadget/Storage/qpe
usbstorageservice.path=/services/UsbGadget/Storage
INSTALLS+=usbstorageservice

usbdesktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/usbconnectionmode.desktop
usbdesktop.path=/apps/Settings
usbdesktop.hint=desktop
INSTALLS+=usbdesktop

usbservicehelp.source=$$QTOPIA_DEPOT_PATH/help
usbservicehelp.files=usbgadget-ethernet-qpe.html usbgadget-storage-qpe.html
usbservicehelp.hint=help
INSTALLS+=usbservicehelp

