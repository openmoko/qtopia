TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= packagemanager.h
SOURCES		= packagemanager.cpp \
		  main.cpp
INTERFACES	= packagemanagerbase.ui pkdesc.ui pkfind.ui pksettings.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= qipkg

TRANSLATIONS	= $(QPEDIR)/i18n/de/qipkg.ts
