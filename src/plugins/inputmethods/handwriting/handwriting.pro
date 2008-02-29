singleprocess:singleprocess=true
TEMPLATE    = lib
CONFIG	    += qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
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
VERSION	    = 1.0.0

TRANSLATIONS = libqhandwriting-en_GB.ts libqhandwriting-de.ts libqhandwriting-ja.ts libqhandwriting-no.ts
