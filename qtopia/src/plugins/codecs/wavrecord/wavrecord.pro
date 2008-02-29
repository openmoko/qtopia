CONFIG		+= qtopiaplugin

TARGET		=  wavrecord

HEADERS		=  wavrecord.h wavrecordimpl.h
SOURCES	        =  wavrecord.cpp wavrecordimpl.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

INCLUDEPATH	+= ../../../3rdparty/libraries/gsm

LIBS    +=  -lgsm
