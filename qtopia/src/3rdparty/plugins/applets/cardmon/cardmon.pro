CONFIG		+= qtopiaplugin 

HEADERS	=	cardmon.h cardmonimpl.h
SOURCES	=	cardmon.cpp cardmonimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

TARGET		= cardmon

pics.files=$${QTOPIA_DEPOT_PATH}/pics/cardmon/*
pics.path=/pics/cardmon
PICS_INSTALLS+=pics

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/applets/cardmonapplet.desktop
desktop.path=/plugins/applets/

INSTALLS+=desktop
