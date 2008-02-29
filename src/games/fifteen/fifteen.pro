multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= fifteen.h
SOURCES		= fifteen.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= fifteen
TRANSLATIONS = fifteen-en_GB.ts fifteen-de.ts fifteen-ja.ts fifteen-no.ts
