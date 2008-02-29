TARGET  = qsvg
include(../../qpluginbase.pri)

VERSION = 1.0.0

HEADERS += qsvgiohandler.h
SOURCES += main.cpp \
           qsvgiohandler.cpp
QT += svg

DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats
target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
