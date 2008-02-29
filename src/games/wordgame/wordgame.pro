TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= wordgame.h
SOURCES		= main.cpp \
		  wordgame.cpp
INTERFACES	= newgamebase.ui rulesbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= wordgame

TRANSLATIONS	= $(QPEDIR)/i18n/de/wordgame.ts
