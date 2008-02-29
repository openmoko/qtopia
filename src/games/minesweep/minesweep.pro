multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= minefield.h \
		  minesweep.h
SOURCES		= minefield.cpp \
		  minesweep.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= minesweep

TRANSLATIONS = minesweep-en_GB.ts minesweep-de.ts minesweep-ja.ts minesweep-no.ts
