TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= settime.h
SOURCES		= settime.cpp main.cpp

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= systemtime

TRANSLATIONS	= $(QPEDIR)/i18n/de/systemtime.ts 
