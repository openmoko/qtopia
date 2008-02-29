multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= amigo.h \
		  go.h \
		  goplayutils.h \
		  gowidget.h
SOURCES		= amigo.c \
		  goplayer.c \
		  goplayutils.c \
		  killable.c \
		  gowidget.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= go

TRANSLATIONS = go-en_GB.ts go-de.ts go-ja.ts go-no.ts
