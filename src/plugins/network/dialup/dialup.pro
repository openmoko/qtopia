TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= dialup.h ../proxiespage.h
SOURCES		= dialup.cpp ../proxiespage.cpp
INTERFACES	= dialupbase.ui ../proxiespagebase_p.ui
TARGET		= dialup
DESTDIR		= $(QPEDIR)/plugins/network
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include 
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS    = $(QPEDIR)/i18n/de/libdialup.ts
