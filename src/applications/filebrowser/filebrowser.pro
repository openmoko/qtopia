TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= inlineedit.h \
		  filebrowser.h
SOURCES		= filebrowser.cpp \
		  inlineedit.cpp \
		  main.cpp

INTERFACES	= 
TARGET		= filebrowser
TRANSLATIONS = filebrowser-en_GB.ts filebrowser-de.ts filebrowser-ja.ts filebrowser-no.ts
