TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= clock.h clockappletimpl.h
SOURCES		= clock.cpp clockappletimpl.cpp
TARGET		= clockapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libclockapplet.ts
