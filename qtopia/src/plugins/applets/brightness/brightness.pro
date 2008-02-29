CONFIG		+= qtopiaplugin

TARGET		= brightnessapplet

HEADERS		= brightness.h brightnessappletimpl.h
SOURCES		= brightness.cpp brightnessappletimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/brightnessapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
