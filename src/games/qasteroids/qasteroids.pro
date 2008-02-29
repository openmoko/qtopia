multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= ledmeter.h  sprites.h  toplevel.h  view.h
SOURCES		= ledmeter.cpp  toplevel.cpp  view.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= qasteroids

TRANSLATIONS = qasteroids-en_GB.ts qasteroids-de.ts qasteroids-ja.ts qasteroids-no.ts
