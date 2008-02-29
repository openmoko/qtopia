TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS	=   accessory.h
SOURCES	=   accessory.cpp

INCLUDEPATH += $(QPEDIR)/include $(QPEDIR)/src/server

INTERFACES = 

TARGET		= qtopiaaccessory
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.0

