singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG	+= dll
win32:DEFINES += QTOPIA_MAKEDLL QT_DLL

# library files
HEADERS = calculator.h engine.h data.h instruction.h stdinputwidgets.h
SOURCES = calculator.cpp engine.cpp instruction.cpp stdinputwidgets.cpp

# data types
HEADERS += doubledata.h integerdata.h fractiondata.h
SOURCES += doubledata.cpp integerdata.cpp fractiondata.cpp

# instructions
HEADERS += doubleinstruction.h integerinstruction.h fractioninstruction.h
SOURCES += doubleinstruction.cpp integerinstruction.cpp fractioninstruction.cpp

static:SOURCES += $(QPEDIR)/src/plugins/calculator/simple/simple.cpp
static:HEADERS += $(QPEDIR)/src/plugins/calculator/simple/simple.h

TARGET		= qtopiacalc
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.6.0

