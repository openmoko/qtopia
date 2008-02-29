TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= dialup.h ../proxiespage.h
SOURCES		= dialup.cpp ../proxiespage.cpp
INTERFACES	= dialupbase.ui ../proxiespagebase_p.ui
TARGET		= dialup
DESTDIR		= $(QPEDIR)/plugins/network
VERSION		= 1.0.0

TRANSLATIONS = libdialup-en_GB.ts libdialup-de.ts libdialup-ja.ts libdialup-no.ts
