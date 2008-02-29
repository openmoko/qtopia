TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= networksettings.h addnetworksetting.h
SOURCES		= main.cpp networksettings.cpp addnetworksetting.cpp
INTERFACES	= addnetworksettingbase.ui networksettingsbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= netsetup

TRANSLATIONS	= $(QPEDIR)/i18n/de/netsetup.ts
