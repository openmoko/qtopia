TEMPLATE	= lib
CONFIG		+= qtopia

# IF YOU MAKE CHANGE HERE TO THE SOURCES
# OR HEADERS, PLEASE UPDATE THE QTOPIA.PRO
# FILE WITH THE SAME CHANGE SINCE
# QTOPIA ON WINDOWS COMPILES IT AS ONE LIBRARY

HEADERS	=   accessory.h \
	    datepicker.h \
	    datetimeedit.h \
	    fieldmapimpl.h \
	    qprocess.h \
	    timezone.h
SOURCES	=   applnk1.cpp \
	    categories1.cpp \
	    categoryselect1.cpp \
	    config1.cpp \
	    qpeapplication1.cpp \
	    storage1.cpp \
	    accessory.cpp \
	    calendar1.cpp \
	    resource1.cpp \
	    datepicker.cpp \
	    datetimeedit.cpp \
	    timeconversion1.cpp \
	    timestring1.cpp \
	    global1.cpp \
	    fieldmapimpl.cpp \
	    qprocess.cpp \
	    timezone.cpp

embedded:HEADERS +=	services.h \
	devicebuttonmanager.h \
	devicebutton.h \
	qwizard.h \
	locationcombo.h \
	docproperties.h \
	pluginloader.h

embedded:SOURCES +=  	   services.cpp \
	devicebuttonmanager.cpp \
	devicebutton.cpp \
	fileselector1.cpp \
	qwizard.cpp \
	locationcombo.cpp \
	docproperties.cpp \
	pluginloader.cpp \
	pluginloaderlib.cpp 


unix:SOURCES += qprocess_unix.cpp
win32:SOURCES += qprocess_win.cpp

qt2:SOURCES += quuid1.cpp

INCLUDEPATH += $(QPEDIR)/src/server

unix:LIBS   += -luuid
win32:LIBS += rpcrt4.lib

TARGET		= qtopia
qdesktop:TARGET = qd-qtopia
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.1

TRANSLATIONS = libqtopia-en_GB.ts libqtopia-de.ts libqtopia-ja.ts libqtopia-no.ts

