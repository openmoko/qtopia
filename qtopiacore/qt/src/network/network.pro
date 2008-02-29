TARGET   = QtNetwork
QPRO_PWD = $$PWD
DEFINES += QT_BUILD_NETWORK_LIB
QT = core
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x64000000

include(../qbase.pri)

# Qt network module

PRECOMPILED_HEADER = ../corelib/global/qt_pch.h
HEADERS += qftp.h \
           qhttp.h \
           qhostaddress.h \
           qabstractsocketengine_p.h \
           qnativesocketengine_p.h \
           qsocks5socketengine_p.h \
           qabstractsocket.h \
           qabstractsocket_p.h \
           qtcpsocket.h \
           qudpsocket.h \
           qtcpserver.h \
           qhostinfo.h \
           qhostinfo_p.h \
           qurlinfo.h \
           qnetworkproxy.h \
	   qnetworkinterface.h \
	   qnetworkinterface_p.h

SOURCES	= qftp.cpp \
          qhttp.cpp \
          qhostaddress.cpp \
          qabstractsocketengine.cpp \
          qnativesocketengine.cpp \
          qsocks5socketengine.cpp \
          qabstractsocket.cpp \
          qtcpsocket.cpp \
          qudpsocket.cpp \
          qtcpserver.cpp \
          qhostinfo.cpp \
          qurlinfo.cpp \
          qnetworkproxy.cpp \
	  qnetworkinterface.cpp

unix:SOURCES += qhostinfo_unix.cpp qnativesocketengine_unix.cpp qnetworkinterface_unix.cpp
win32:SOURCES += qhostinfo_win.cpp qnativesocketengine_win.cpp qnetworkinterface_win.cpp


mac:INCLUDEPATH += ../3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)

QMAKE_LIBS += $$QMAKE_LIBS_NETWORK
