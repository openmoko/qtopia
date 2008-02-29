CONFIG		+= qtopiaplugin

TARGET		= freetypefactory

INCLUDEPATH     += $${QTEDIR}/src/3rdparty/freetype/include $${QTEDIR}/src/3rdparty/freetype/include/freetype/config

LIBS+=-lqt-freetype

HEADERS		= fontfactoryttf_qws.h freetypefactoryimpl.h
SOURCES		= fontfactoryttf_qws.cpp freetypefactoryimpl.cpp
qt2:HEADERS	+= qfontdata_p.h

PACKAGE_NAME	= qpe-freetype

TRANSLATIONS=
