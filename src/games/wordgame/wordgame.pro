multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
#singleprocess:TEMPLATE	= lib
#singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= wordgame.h
SOURCES		= wordgame.cpp
INTERFACES	= newgamebase.ui rulesbase.ui

multiprocess:SOURCES+=main.cpp

TARGET		= wordgame

TRANSLATIONS = wordgame-en_GB.ts wordgame-de.ts wordgame-ja.ts wordgame-no.ts
