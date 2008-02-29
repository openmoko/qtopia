CONFIG		+= qtopiaplugin

TARGET		= irreceiverapplet

HEADERS		= irreceiver.h irreceiverappletimpl.h ircontroller.h
SOURCES		= irreceiver.cpp irreceiverappletimpl.cpp ircontroller.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/irreceiverapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
