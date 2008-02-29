SOURCES = qdbus.cpp
DESTDIR = ../../../../bin
TARGET = qdbus
QT = core
CONFIG += qdbus
CONFIG -= app_bundle

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
