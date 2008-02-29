singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_MAKEDLL QTOPIA_PLUGIN_MAKEDLL
win32:CONFIG += dll 

# IF YOU MAKE CHANGE HERE TO THE SOURCES
# OR HEADERS, PLEASE UPDATE THE QTOPIA.PRO
# FILE WITH THE SAME CHANGE SINCE
# QTOPIA ON WINDOWS COMPILES IT AS ONE LIBRARY

HEADERS	=   accessory.h \
	    datepicker.h \
	    datetimeedit.h \
	    fieldmapimpl.h
SOURCES	=   applnk1.cpp \
	    categories1.cpp \
	    categoryselect1.cpp \
	    qpeapplication1.cpp \
	    storage1.cpp \
	    accessory.cpp \
	    calendar1.cpp \
	    resource1.cpp \
	    datepicker.cpp \
	    datetimeedit.cpp \
	    global1.cpp \
	    fieldmapimpl.cpp

embedded:HEADERS +=	services.h \
	devicebuttonmanager.h \
	devicebutton.h \
	qwizard.h \
	docproperties.h

embedded:SOURCES +=  	   services.cpp \
	devicebuttonmanager.cpp \
	devicebutton.cpp \
	fileselector1.cpp \
	qwizard.cpp \
	docproperties.cpp

INCLUDEPATH += $(QPEDIR)/src/server

unix:LIBS   += -luuid
win32:LIBS += rpcrt4.lib

TARGET		= qtopia
qdesktop:TARGET		= qd-qtopia
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.0

TRANSLATIONS = libqtopia-en_GB.ts libqtopia-de.ts libqtopia-ja.ts libqtopia-no.ts
