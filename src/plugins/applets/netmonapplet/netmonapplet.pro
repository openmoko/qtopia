TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= netmon.h netmonappletimpl.h
SOURCES		= netmon.cpp netmonappletimpl.cpp
TARGET		= netmonapplet
INTERFACES	= netpw.ui
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libnetmonapplet.ts
