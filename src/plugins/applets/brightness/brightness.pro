TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

HEADERS		= brightness.h brightnessappletimpl.h
SOURCES		= brightness.cpp brightnessappletimpl.cpp
TARGET		= brightnessapplet
DESTDIR		= $(QPEDIR)/plugins/applets
VERSION		= 1.0.0



