TEMPLATE        = app
CONFIG          += qtopia warn_on release
DESTDIR         = $(QPEDIR)/bin

HEADERS		= mindbreaker.h
SOURCES		= main.cpp \
		  mindbreaker.cpp

TARGET          = mindbreaker
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe

TRANSLATIONS	= $(QPEDIR)/i18n/de/mindbreaker.ts
