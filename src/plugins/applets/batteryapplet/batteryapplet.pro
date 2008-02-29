TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= battery.h batterystatus.h batteryappletimpl.h
SOURCES		= battery.cpp batterystatus.cpp batteryappletimpl.cpp
TARGET		= batteryapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include ..
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libbatteryapplet.ts
