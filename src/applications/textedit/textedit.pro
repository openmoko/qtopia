TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= textedit.h
SOURCES		= main.cpp textedit.cpp

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= textedit

TRANSLATIONS	= $(QPEDIR)/i18n/de/textedit.ts
