TARGET = qdecorationstyled
include(../../qpluginbase.pri)

target.path += $$[QT_INSTALL_PLUGINS]/decorations
INSTALLS += target

DEFINES += QT_QWS_DECORATION_STYLED

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qdecorationstyled_qws.h
SOURCES	= main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qdecorationstyled_qws.cpp

