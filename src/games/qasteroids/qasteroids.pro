TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= ledmeter.h  sprites.h  toplevel.h  view.h ../../server/devicebuttonmanager.h ../../server/devicebutton.h
SOURCES		= ledmeter.cpp  toplevel.cpp  view.cpp main.cpp ../../server/devicebuttonmanager.cpp ../../server/devicebutton.cpp

DEPENDPATH	+= $(QPEDIR)/include
INCLUDEPATH	+= $(QPEDIR)/include
LIBS            += -lqtopiaservices -lqpe

TARGET		= qasteroids

TRANSLATIONS	= $(QPEDIR)/i18n/de/qasteroids.ts
