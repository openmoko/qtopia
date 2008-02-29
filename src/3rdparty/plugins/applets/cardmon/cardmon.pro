TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS	=	cardmon.h cardmonimpl.h
SOURCES	=	cardmon.cpp cardmonimpl.cpp
TARGET		= cardmonapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libcardmonapplet-en_GB.ts libcardmonapplet-de.ts libcardmonapplet-ja.ts libcardmonapplet-no.ts
