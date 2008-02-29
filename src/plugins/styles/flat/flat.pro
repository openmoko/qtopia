TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= flat.h
SOURCES		= flat.cpp
TARGET		= flatstyle
DESTDIR		= $(QPEDIR)/plugins/styles
VERSION		= 1.0.0

