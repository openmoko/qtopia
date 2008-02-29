singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG	+= dll
win32:DEFINES += QTOPIA_MAKEDLL QT_DLL

# library files
HEADERS = calculator.h engine.h data.h instruction.h stdinputwidgets.h
SOURCES = calculator.cpp engine.cpp data.cpp instruction.cpp stdinputwidgets.cpp

# data types
HEADERS += doubledata.h
SOURCES += doubledata.cpp

# instructions
HEADERS += doubleinstruction.h
SOURCES += doubleinstruction.cpp

static:SOURCES += $(QPEDIR)/src/plugins/calculator/simple/simple.cpp
static:HEADERS += $(QPEDIR)/src/plugins/calculator/simple/simple.h
static:LIBS	+= -lqpe

TARGET		= qtopiacalc
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
win32:DLLDESTDIR = $(QPEDIR)/bin
VERSION		= 1.7.0

