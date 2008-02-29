TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= addressbook.h \
                  abeditor.h \
		  ablabel.h \
		  abtable.h \
		  addresssettings.h
SOURCES		= main.cpp \
		  addressbook.cpp \
		  abeditor.cpp \
		  ablabel.cpp \
		  abtable.cpp \
		  addresssettings.cpp
INTERFACES	= addresssettingsbase.ui

TARGET		= addressbook
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lqpepim

TRANSLATIONS = $(QPEDIR)/i18n/de/addressbook.ts
