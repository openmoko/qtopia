TARGET	 = qkrcodecs
include(../../qpluginbase.pri)

CONFIG	+= warn_on
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs
QT = core

HEADERS		= qeuckrcodec.h

SOURCES		= qeuckrcodec.cpp \
		  main.cpp

target.path += $$[QT_INSTALL_PLUGINS]/codecs
INSTALLS += target
