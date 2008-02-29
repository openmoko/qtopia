CONFIG		+= qtopiaplugin 

HEADERS		= battery.h batterystatus.h batteryappletimpl.h
SOURCES		= battery.cpp batterystatus.cpp batteryappletimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET		= batteryapplet

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/batteryapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
