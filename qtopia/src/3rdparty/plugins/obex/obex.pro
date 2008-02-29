CONFIG		+= qtopiaplugin
CONFIG          -= warn_on
QTOPIA_PROJECT_TYPE = obex

HEADERS		= qir.h qirserver.h qobex.h receivedialog.h sendwindow.h
SOURCES		= qir.cpp qirserver.cpp qobex.cpp receivedialog.cpp sendwindow.cpp
INTERFACES 	+= receivedialogbase.ui sendwidgetbase.ui

TRANSLATABLES   = $${HEADERS} $${SOURCES} $${INTERFACES}

INCLUDEPATH	+= ./openobex 
LIBS	+= -lopenobex

TARGET		= qobex
PACKAGE_NAME = qpe-obex

