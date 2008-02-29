TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= security.h
SOURCES		= security.cpp main.cpp
INTERFACES	= securitybase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= security

TRANSLATIONS   = $(QPEDIR)/i18n/de/security.ts 
