TEMPLATE	= lib
CONFIG		+= qtopia warn_on release

# library files
HEADERS = engine.h data.h instruction.h stdinputwidgets.h
SOURCES = engine.cpp instruction.cpp stdinputwidgets.cpp

# data types
HEADERS += doubledata.h integerdata.h fractiondata.h
SOURCES += doubledata.cpp integerdata.cpp fractiondata.cpp

# instructions
HEADERS += doubleinstruction.h integerinstruction.h fractioninstruction.h
SOURCES += doubleinstruction.cpp integerinstruction.cpp fractioninstruction.cpp

INCLUDEPATH += $(QPEDIR)/include

TARGET		= qtopiacalc
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.0

