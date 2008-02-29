TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= irreceiver.h irreceiverappletimpl.h
SOURCES		= irreceiver.cpp irreceiverappletimpl.cpp
TARGET		= irreceiverapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libirreceiverapplet-en_GB.ts libirreceiverapplet-de.ts libirreceiverapplet-ja.ts libirreceiverapplet-no.ts
