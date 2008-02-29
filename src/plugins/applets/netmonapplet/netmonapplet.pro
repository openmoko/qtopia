TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= netmon.h netmonappletimpl.h
SOURCES		= netmon.cpp netmonappletimpl.cpp
TARGET		= netmonapplet
INTERFACES	= netpw.ui
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libnetmonapplet-en_GB.ts libnetmonapplet-de.ts libnetmonapplet-ja.ts libnetmonapplet-no.ts
