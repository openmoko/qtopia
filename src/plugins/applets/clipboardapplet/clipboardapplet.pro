TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
win32:CONFIG += dll
HEADERS		= clipboard.h clipboardappletimpl.h
SOURCES		= clipboard.cpp clipboardappletimpl.cpp
TARGET		= clipboardapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0

TRANSLATIONS = libclipboardapplet-en_GB.ts libclipboardapplet-de.ts libclipboardapplet-ja.ts libclipboardapplet-no.ts
