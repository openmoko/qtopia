multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
#singleprocess:TEMPLATE	= lib
#singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= settings.h
SOURCES		= light.cpp
INTERFACES	= lightsettingsbase.ui

multiprocess:SOURCES+=main.cpp

TARGET		= light-and-power

TRANSLATIONS = light-and-power-en_GB.ts light-and-power-de.ts light-and-power-ja.ts light-and-power-no.ts
