TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= inlineedit.h \
		  filebrowser.h
SOURCES		= filebrowser.cpp \
		  inlineedit.cpp \
		  main.cpp
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lqtopiaservices
INTERFACES	= 
