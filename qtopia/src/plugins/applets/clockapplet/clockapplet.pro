CONFIG		+= qtopiaplugin 

TARGET		= clockapplet

HEADERS		= clock.h clockappletimpl.h
SOURCES		= clock.cpp clockappletimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/clockapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
