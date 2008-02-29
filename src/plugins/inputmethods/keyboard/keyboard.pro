TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= keyboard.h \
		    pickboardcfg.h \
		    pickboardpicks.h \
		    keyboardimpl.h 
SOURCES		= keyboard.cpp \
		    pickboardcfg.cpp \
		    pickboardpicks.cpp \
		    keyboardimpl.cpp
TARGET		= qkeyboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libqkeyboard.ts
