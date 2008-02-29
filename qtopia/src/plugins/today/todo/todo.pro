CONFIG += qtopiaplugin 

TARGET = todoplugin

HEADERS = todoplugin.h todopluginimpl.h todopluginoptions.h
SOURCES = todoplugin.cpp todopluginimpl.cpp todopluginoptions.cpp
INTERFACES = todooptionsbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}
INCLUDEPATH     += ../ ../library
DEPENDPATH      += ../ ../library

CONFIG+=pimlib

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/today/todoplugin.desktop
desktop.path=/plugins/today/

INSTALLS+=desktop

PACKAGE_NAME	= qpe-today-todoplugin
