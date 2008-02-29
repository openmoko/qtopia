TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= appservices.h
SOURCES		= appservices.cpp main.cpp

INTERFACES	= appservicesbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lqtopiaservices

TARGET		= appservices

TRANSLATIONS    = $(QPEDIR)/i18n/de/appservices.ts
