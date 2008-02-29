TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= 
SOURCES		= main.cpp
INTERFACES	= 

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe

TARGET		= qcop
