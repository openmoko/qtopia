TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= polished.h
SOURCES		= polished.cpp
TARGET		= polished
DESTDIR		= $(QPEDIR)/plugins/decorations
VERSION		= 1.0.0

TRANSLATIONS = libpolished-en_GB.ts libpolished-de.ts libpolished-ja.ts libpolished-no.ts
