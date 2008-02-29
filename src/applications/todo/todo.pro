multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG		+= qtopia warn_on release
HEADERS		= mainwindow.h \
		  todotable.h \
		  todoentryimpl.h
SOURCES		= mainwindow.cpp \
		  todotable.cpp \
		  todoentryimpl.cpp

multiprocess:SOURCES+=main.cpp

INTERFACES	= taskdetail.ui

TARGET		= todolist

unix:LIBS       += -lqpepim
win32:LIBS      += $(QPEDIR)/lib/qpepim.lib

TRANSLATIONS = todolist-en_GB.ts todolist-de.ts todolist-ja.ts todolist-no.ts
