TEMPLATE    = app
CONFIG	    += qtopia warn_on release
DESTDIR	    = $(QPEDIR)/bin

HEADERS	    = buttoneditordialog.h  

SOURCES	    = main.cpp \
	      buttoneditordialog.cpp 

INTERFACES  = buttonrow.ui

TARGET	    = buttoneditor
TRANSLATIONS = buttoneditor-en_GB.ts buttoneditor-de.ts buttoneditor-ja.ts buttoneditor-no.ts
