TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = datebookplugin.h datebookpluginimpl.h datebookpluginoptions.h
SOURCES = datebookplugin.cpp datebookpluginimpl.cpp datebookpluginoptions.cpp
INTERFACES = datebookoptionsbase.ui

INCLUDEPATH     += ../ ../library
DEPENDPATH      += ../ ../library

unix:LIBS+= -lqpepim
win32:LIBS+= $(QPEDIR)/lib/qpepim.lib

DESTDIR = $(QPEDIR)/plugins/today
TARGET = datebookplugin

TRANSLATIONS = libdatebookplugin-en_GB.ts libdatebookplugin-de.ts libdatebookplugin-ja.ts libdatebookplugin-no.ts
