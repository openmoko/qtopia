multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= settime.h
SOURCES		= settime.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= systemtime

TRANSLATIONS = systemtime-en_GB.ts systemtime-de.ts systemtime-ja.ts systemtime-no.ts
