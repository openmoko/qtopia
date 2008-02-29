TARGET  = qsvg
include(../../qpluginbase.pri)

VERSION = 1.0.0

HEADERS += qsvgiconengine.h
SOURCES += main.cpp \
           qsvgiconengine.cpp
QT += svg

DESTDIR  = $$QT_BUILD_TREE/plugins/iconengines
target.path += $$[QT_INSTALL_PLUGINS]/iconengines
INSTALLS += target
