TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= settings.h
SOURCES		= light.cpp main.cpp
INTERFACES	= lightsettingsbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= light-and-power

TRANSLATIONS	= #(QPEDIR)/i18n/de/light-and-power.ts
