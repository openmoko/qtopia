TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= amigo.h \
		  go.h \
		  goplayutils.h \
		  gowidget.h
SOURCES		= amigo.c \
		  goplayer.c \
		  goplayutils.c \
		  killable.c \
		  gowidget.cpp \
		  main.cpp

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= go

TRANSLATIONS	= $(QPEDIR)/i18n/de/go.ts
