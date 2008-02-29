TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS	=	cardmon.h cardmonimpl.h
SOURCES	=	cardmon.cpp cardmonimpl.cpp
TARGET		= cardmonapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	= $(QPEDIR)/i18n/de/libcardmonapplet.ts
