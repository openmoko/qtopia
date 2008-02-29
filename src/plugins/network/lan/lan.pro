TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= lan.h ../proxiespage.h
SOURCES		= lan.cpp ../proxiespage.cpp
INTERFACES	= lanbase.ui lanstate.ui ../proxiespagebase_p.ui
TARGET		= lan
DESTDIR		= $(QPEDIR)/plugins/network
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS    = $(QPEDIR)/i18n/de/liblan.ts
