TARGET   = qtwcodecs
include(../../qpluginbase.pri)

CONFIG  += warn_on
DESTDIR  = $$QT_BUILD_TREE/plugins/codecs
QT = core

HEADERS  = qbig5codec.h

SOURCES  = qbig5codec.cpp \
	   main.cpp

target.path += $$[QT_INSTALL_PLUGINS]/codecs
INSTALLS += target
