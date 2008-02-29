TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= lan.h ../proxiespage.h
SOURCES		= lan.cpp ../proxiespage.cpp
INTERFACES	= lanbase.ui lanstate.ui ../proxiespagebase_p.ui
win32:INCLUDEPATH += .
TARGET		= lan
DESTDIR		= $(QPEDIR)/plugins/network
VERSION		= 1.0.0

TRANSLATIONS = liblan-en_GB.ts liblan-de.ts liblan-ja.ts liblan-no.ts
