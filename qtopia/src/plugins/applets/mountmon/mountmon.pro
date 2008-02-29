CONFIG		+= qtopiaplugin

TARGET		= mountmonapplet

HEADERS	=	mountmon.h mountmonimpl.h
SOURCES	=	mountmon.cpp mountmonimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

pics.files=$${QTOPIA_DEPOT_PATH}/pics/mountmon/*
pics.path=/pics/mountmon
PICS_INSTALLS+=pics

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/mountmonapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
