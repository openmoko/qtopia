singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= clock.h clockappletimpl.h
SOURCES		= clock.cpp clockappletimpl.cpp
TARGET		= clockapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libclockapplet-en_GB.ts libclockapplet-de.ts libclockapplet-ja.ts libclockapplet-no.ts
