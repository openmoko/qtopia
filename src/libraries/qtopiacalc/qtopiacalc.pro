TEMPLATE	= lib
CONFIG		+= qtopia singleexec

TARGET		= qtopiacalc
DESTDIR		= $$(QPEDIR)/lib

# library files
HEADERS = calculator.h engine.h data.h instruction.h display.h stdinputwidgets.h\
	integerdata.h integerinstruction.h plugininterface.h qtopiacalcwinexport.h
SOURCES = calculator.cpp engine.cpp data.cpp instruction.cpp display.cpp stdinputwidgets.cpp

# double type
HEADERS += doubledata.h
SOURCES += doubledata.cpp

PHONEPATH = ../../plugins/calculator/phone
SIMPLEPATH = ../../plugins/calculator/simple

FRACTION_HEADERS = fractiondata.h fractioninstruction.h doubleinstruction.h
FRACTION_SOURCES = fractiondata.cpp fractioninstruction.cpp doubleinstruction.cpp
PHONE_HEADERS = $${PHONEPATH}/phone.h $${PHONEPATH}/instructions.h \
                doubleinstruction.h $${SIMPLEPATH}/simple.h
PHONE_SOURCES = $${PHONEPATH}/phone.cpp $${PHONEPATH}/instructions.cpp \
                doubleinstruction.cpp $${SIMPLEPATH}/simple.cpp

TRANSLATABLES = $${HEADERS} \
                $${FRACTION_HEADERS} \
                $${SOURCES} \
                $${FRACTION_SOURCES} \
                $${PHONE_HEADERS} \
                $${PHONE_SOURCES}
!QTOPIA_PHONE {
    DEFINES+=ENABLE_FRACTION
    HEADERS += $${FRACTION_HEADERS}
    SOURCES += $${FRACTION_SOURCES}
} else {
    CONFIG+=staticlib

    VPATH+=$${PHONEPATH} $${SIMPLEPATH}
    HEADERS+= $${PHONE_HEADERS}
    SOURCES+= $${PHONE_SOURCES}
}

