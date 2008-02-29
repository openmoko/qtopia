CONFIG		+= qtopiaplugin

TARGET		= clipboardapplet

HEADERS		= clipboard.h clipboardappletimpl.h
SOURCES		= clipboard.cpp clipboardappletimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/clipboardapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
