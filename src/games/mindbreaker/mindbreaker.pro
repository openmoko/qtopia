multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG          += qtopia warn_on release

HEADERS		= mindbreaker.h
SOURCES		= mindbreaker.cpp
multiprocess:SOURCES+=main.cpp

TARGET          = mindbreaker

TRANSLATIONS = mindbreaker-en_GB.ts mindbreaker-de.ts mindbreaker-ja.ts mindbreaker-no.ts
