TARGET = qvr41xxmousedriver
include(../../qpluginbase.pri)

target.path = $$[QT_INSTALL_PLUGINS]/mousedrivers
INSTALLS += target

DEFINES += QT_QWS_MOUSE_VR41XX

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qmousevr41xx_qws.h

SOURCES	= main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qmousevr41xx_qws.cpp

