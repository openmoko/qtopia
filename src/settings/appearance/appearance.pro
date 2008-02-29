TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= appearance.h
SOURCES		= appearance.cpp main.cpp
INTERFACES	= appearancesettingsbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= appearance

TRANSLATIONS    = $(QPEDIR)/i18n/de/appearance.ts
