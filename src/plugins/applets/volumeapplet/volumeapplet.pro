CONFIG		+= qtopiaplugin

TARGET		= volumeapplet

HEADERS		= volume.h volumeappletimpl.h
SOURCES		= volume.cpp volumeappletimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/volumeapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
