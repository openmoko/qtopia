TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= 
SOURCES		= quickexec.cpp

LIBS		+= -ldl

TARGET		= quickexec
