CONFIG += qtopiaplugin 

TARGET = datebookplugin

HEADERS = datebookplugin.h datebookpluginimpl.h datebookpluginoptions.h
SOURCES = datebookplugin.cpp datebookpluginimpl.cpp datebookpluginoptions.cpp
INTERFACES = datebookoptionsbase.ui

TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

INCLUDEPATH     += ../ ../library
DEPENDPATH      += ../ ../library

CONFIG+=pimlib

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/today/datebookplugin.desktop
desktop.path=/plugins/today/

INSTALLS+=desktop

PACKAGE_NAME	= qpe-today-datebookplugin
