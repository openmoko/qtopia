multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
#singleprocess:TEMPLATE	= lib
#singleprocess:DESTDIR   = $(QPEDIR)/lib

TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= interface.h man.h cannon.h base.h bullet.h helicopter.h
SOURCES		= interface.cpp man.cpp cannon.cpp base.cpp bullet.cpp helicopter.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= parashoot

TRANSLATIONS = parashoot-en_GB.ts parashoot-de.ts parashoot-ja.ts parashoot-no.ts
