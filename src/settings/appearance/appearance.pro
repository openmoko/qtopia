multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= appearance.h
SOURCES		= appearance.cpp
INTERFACES	= appearancesettingsbase.ui

multiprocess:SOURCES+=main.cpp

TARGET		= appearance

TRANSLATIONS = appearance-en_GB.ts appearance-de.ts appearance-ja.ts appearance-no.ts
