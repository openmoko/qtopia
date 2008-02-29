TEMPLATE    = lib
CONFIG	    += qtopia warn_on release
HEADERS	    = qimpenchar.h \
		qimpenprofile.h \
		qimpencombining.h \
		qimpenhelp.h \
		qimpeninput.h \
		qimpenmatch.h \
		qimpensetup.h \
		qimpenstroke.h \
		qimpenwidget.h \
		qimpenwordpick.h \
		handwritingimpl.h
SOURCES	    = qimpenchar.cpp \
		qimpenprofile.cpp \
		qimpencombining.cpp \
		qimpenhelp.cpp \
		qimpeninput.cpp \
		qimpenmatch.cpp \
		qimpensetup.cpp \
		qimpenstroke.cpp \
		qimpenwidget.cpp \
		qimpenwordpick.cpp \
		handwritingimpl.cpp
INTERFACES  = qimpenprefbase.ui
TARGET	    = qhandwriting
DESTDIR	    = $(QPEDIR)/plugins/inputmethods
INCLUDEPATH += $(QPEDIR)/include
DEPENDPATH  += $(QPEDIR)/include
LIBS	    += -lqpe
VERSION	    = 1.0.0

TRANSLATIONS += $(QPEDIR)/i18n/de/libqhandwriting.ts
