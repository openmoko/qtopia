TEMPLATE = lib
CONFIG -=
CONFIG += qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = todoplugin.h todopluginimpl.h todopluginoptions.h
SOURCES = todoplugin.cpp todopluginimpl.cpp todopluginoptions.cpp
INTERFACES = todooptionsbase.ui

INCLUDEPATH     += ../ ../library
DEPENDPATH      += ../ ../library

unix:LIBS    += -lqpepim
win32:LIBS   += $(QPEDIR)/lib/qpepim.lib

DESTDIR = $(QPEDIR)/plugins/today
TARGET = todoplugin

TRANSLATIONS = libtodoplugin-en_GB.ts libtodoplugin-de.ts libtodoplugin-ja.ts libtodoplugin-no.ts
