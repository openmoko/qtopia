TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= flat.h
SOURCES		= flat.cpp
TARGET		= flat
DESTDIR		= $(QPEDIR)/plugins/decorations
VERSION		= 1.0.0

TRANSLATIONS = libflat-en_GB.ts libflat-de.ts libflat-ja.ts libflat-no.ts
