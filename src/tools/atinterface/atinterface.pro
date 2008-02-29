qtopia_project(qtopia core app)
TARGET=atinterface
CONFIG+=no_tr singleexec_main

HEADERS	= \
    atcallmanager.h \
    atcommands.h \
    atcustom.h \
    atfrontend.h \
    atindicators.h \
    atinterface.h \
    atoptions.h \
    atsessionmanager.h \
    modememulatorservice.h

SOURCES	= \
    main.cpp \
    atcallmanager.cpp \
    atcommands.cpp \
    atfrontend.cpp \
    atindicators.cpp \
    atinterface.cpp \
    atoptions.cpp \
    atsessionmanager.cpp \
    modememulatorservice.cpp

depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)
depends(libraries/qtopiacomm/serial)

pkg.desc=AT interface for remote access to the device over a cable
pkg.domain=window,phonecomm,bluetooth,input,rfcomm

modememulservice.files=$$QTOPIA_DEPOT_PATH/services/ModemEmulator/atinterface
modememulservice.path=/services/ModemEmulator
INSTALLS+=modememulservice
