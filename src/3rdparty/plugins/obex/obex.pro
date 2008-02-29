TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/plugins/obex/
VERSION		= 1.0.0

HEADERS		= qir.h qirserver.h qobex.h receivedialog.h senddialog.h
SOURCES		= qir.cpp qirserver.cpp qobex.cpp receivedialog.cpp senddialog.cpp
INTERFACES 	+= receivedialogbase.ui senddialogbase.ui


INCLUDEPATH	+= ./openobex $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS		+= -lopenobex -lqtopiaservices -lqpe

TARGET		= qobex

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libqobex.ts
