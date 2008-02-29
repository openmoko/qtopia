TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= snake.h target.h obstacle.h interface.h codes.h border.h
SOURCES		= snake.cpp target.cpp obstacle.cpp interface.cpp main.cpp \
		    border.cpp


INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= snake

TRANSLATIONS	= $(QPEDIR)/i18n/de/snake.ts
