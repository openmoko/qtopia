!qbuild{
qtopia_project(qtopia core app)
TARGET=atinterface
CONFIG+=no_tr singleexec_main

enable_cell:depends(libraries/qtopiamail)
depends(libraries/qtopiaphone)
enable_cell:depends(libraries/qtopiapim)
depends(libraries/qtopiacomm/serial)
depends(libraries/qtopiacomm/usb)
}

HEADERS	= \
    atcustom.h \
    atcallmanager.h \
    atparseutils.h \
    atcommands.h \
    atgsmnoncellcommands.h \
    atv250commands.h \
    atfrontend.h \
    atindicators.h \
    atinterface.h \
    atoptions.h \
    atsessionmanager.h \
    modememulatorservice.h

SOURCES	= \
    main.cpp \
    atcallmanager.cpp \
    atparseutils.cpp \
    atcommands.cpp \
    atgsmnoncellcommands.cpp \
    atv250commands.cpp \
    atfrontend.cpp \
    atindicators.cpp \
    atinterface.cpp \
    atoptions.cpp \
    atsessionmanager.cpp \
    modememulatorservice.cpp

enable_bluetooth {
    HEADERS	+= \
        atbluetoothcommands.h
    SOURCES	+= \
        atbluetoothcommands.cpp
}

enable_cell {
    HEADERS	+= \
        atgsmcellcommands.h\
        atsmscommands.h

    SOURCES	+= \
        atgsmcellcommands.cpp\
        atsmscommands.cpp
}

pkg.desc=AT interface for remote access to the device over a cable
pkg.domain=trusted

modememulservice.files=$$QTOPIA_DEPOT_PATH/services/ModemEmulator/atinterface
modememulservice.path=/services/ModemEmulator
INSTALLS+=modememulservice

usbservice.files=$$QTOPIA_DEPOT_PATH/services/UsbGadget/Serial/atinterface
usbservice.path=/services/UsbGadget/Serial
INSTALLS+=usbservice

usbservicehelp.source=$$QTOPIA_DEPOT_PATH/help
usbservicehelp.files=usbgadget-serial-atinterface.html
usbservicehelp.hint=help
INSTALLS+=usbservicehelp
