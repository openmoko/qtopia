TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= battery.h batterystatus.h batteryappletimpl.h
SOURCES		= battery.cpp batterystatus.cpp batteryappletimpl.cpp
TARGET		= batteryapplet
DESTDIR		= $(QPEDIR)/plugins/applets
DEPENDPATH      += ..
VERSION		= 1.0.0

win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
win32:CONFIG += dll

TRANSLATIONS = libbatteryapplet-en_GB.ts libbatteryapplet-de.ts libbatteryapplet-ja.ts libbatteryapplet-no.ts
