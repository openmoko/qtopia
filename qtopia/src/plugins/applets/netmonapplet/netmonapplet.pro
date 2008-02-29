CONFIG		+= qtopiaplugin

TARGET		= netmonapplet

HEADERS		= netmon.h netmonappletimpl.h
SOURCES		= netmon.cpp netmonappletimpl.cpp

INTERFACES	= netpw.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

pics.files=$${QTOPIA_DEPOT_PATH}/pics/netmon/*
pics.path=/pics/netmon
PICS_INSTALLS+=pics

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/netmonapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
