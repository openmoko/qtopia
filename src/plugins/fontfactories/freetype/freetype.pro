TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= fontfactoryttf_qws.h freetypefactoryimpl.h
SOURCES		= fontfactoryttf_qws.cpp freetypefactoryimpl.cpp
qt2:HEADERS	+= qfontdata_p.h
TARGET		= freetypefactory

DESTDIR		= $(QPEDIR)/plugins/fontfactories
INCLUDEPATH	+= $(QTDIR)/src/3rdparty/freetype/include $(QTDIR)/src/3rdparty/freetype/include/freetype/config

unix:LIBS       += -lfreetype
win32:LIBS      += $(QPEDIR)/lib/freetype.lib

VERSION		= 1.0.0
