multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= snake.h target.h obstacle.h interface.h codes.h border.h
SOURCES		= snake.cpp target.cpp obstacle.cpp interface.cpp border.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= snake

TRANSLATIONS = snake-en_GB.ts snake-de.ts snake-ja.ts snake-no.ts
