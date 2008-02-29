TARGET	 = qgfxvnc
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VNC

HEADERS		= ../../../../include/Qt/qscreenvnc_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qscreenvnc_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS 	+= target
