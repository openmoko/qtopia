CONFIG		+= qtopia warn_on release
TEMPLATE	= app
DESTDIR		= $(QPEDIR)/bin

HEADERS		= worldtime.h zonemap.h sun.h stylusnormalizer.h
SOURCES		= worldtime.cpp zonemap.cpp main.cpp sun.c stylusnormalizer.cpp
INTERFACES	= worldtimebase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS		+= -lqpe

TARGET		= worldtime

TRANSLATIONS	= $(QPEDIR)/i18n/de/worldtime.ts
