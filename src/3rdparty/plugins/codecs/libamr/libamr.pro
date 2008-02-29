CONFIG		+= qtopiaplugin

TARGET		=  amrrecordplugin

HEADERS		=  amrrecord.h amrrecordimpl.h
SOURCES	        =  amrrecord.cpp amrrecordimpl.cpp

TRANSLATABLES   = $${HEADERS} $${SOURCES}

INCLUDEPATH	+= ../../../../3rdparty/libraries/amr
DEFINES         += MMS_IO

LIBS            +=  -lamr
