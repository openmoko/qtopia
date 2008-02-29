CONFIG		+= qtopiaplugin 

TARGET		=  wavplugin

HEADERS		=  wavplugin.h wavpluginimpl.h
SOURCES	        =  wavplugin.cpp wavpluginimpl.cpp

INCLUDEPATH	+= ../../../3rdparty/libraries/gsm
LIBS		+= -lgsm

TRANSLATIONS=
