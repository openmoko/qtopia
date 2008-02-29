TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= mainwindow.h \
		  todotable.h \
		  todoentryimpl.h
SOURCES		= main.cpp \
		  mainwindow.cpp \
		  todotable.cpp \
		  todoentryimpl.cpp

INTERFACES	= todoentry.ui

TARGET		= todolist
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lqpepim

TRANSLATIONS	= $(QPEDIR)/i18n/de/todolist.ts
