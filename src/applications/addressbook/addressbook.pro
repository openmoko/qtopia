multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR	= $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG		+= qtopia warn_on release
HEADERS		= addressbook.h \
                  abeditor.h \
		  ablabel.h \
		  abtable.h \
		  emaildlgimpl.h
SOURCES		= addressbook.cpp \
		  abeditor.cpp \
		  ablabel.cpp \
		  abtable.cpp \
		  emaildlgimpl.cpp

multiprocess:SOURCES+=main.cpp

INTERFACES	= emaildlg.ui

TARGET		= addressbook

unix:LIBS       += -lqpepim
win32:LIBS      += $(QPEDIR)/lib/qpepim.lib

TRANSLATIONS = addressbook-en_GB.ts addressbook-de.ts addressbook-ja.ts addressbook-no.ts
