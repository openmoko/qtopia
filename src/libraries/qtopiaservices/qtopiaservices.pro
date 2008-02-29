TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS	=   services.h
SOURCES	=   services.cpp

INCLUDEPATH += $(QPEDIR)/include
LIBS		+= -lqpe

TARGET		= qtopiaservices
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.0

