TARGET  = qgif
include(../../qpluginbase.pri)

VERSION = 1.0.0

HEADERS += qgifhandler.h

SOURCES += main.cpp \
           qgifhandler.cpp

DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats
target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
