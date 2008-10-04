TARGET          = QtDBus
QPRO_PWD        = $$PWD
QT              = core xml
CONFIG         += link_pkgconfig
DEFINES        += QDBUS_MAKEDLL DBUS_API_SUBJECT_TO_CHANGE
QMAKE_CXXFLAGS += $$QT_CFLAGS_DBUS

contains(QT_CONFIG, dbus-linked) {
    LIBS       += $$QT_LIBS_DBUS
    DEFINES    += QT_LINKED_LIBDBUS
}

#INCLUDEPATH += .

unix {
   QMAKE_PKGCONFIG_DESCRIPTION = Qt DBus module
}

win32 {
   LIBS += -lws2_32 -ladvapi32 -lnetapi32 -luser32
   CONFIG(debug, debug|release):LIBS += -ldbus-1d
   else:LIBS += -ldbus-1
}

include(../qbase.pri)

PUB_HEADERS =  qdbusargument.h \
	       qdbusconnectioninterface.h \
	       qdbusmacros.h \
	       qdbuserror.h \
	       qdbusextratypes.h \
	       qdbusmessage.h \
	       qdbusserver.h \
	       qdbusconnection.h \
	       qdbusabstractinterface.h \
	       qdbusinterface.h \
	       qdbusabstractadaptor.h \
	       qdbusreply.h \
	       qdbusmetatype.h \
	       qdbuscontext.h

HEADERS += $$PUB_HEADERS \
           qdbusconnection_p.h qdbusmessage_p.h \
           qdbusinterface_p.h qdbusxmlparser_p.h qdbusabstractadaptor_p.h \
           qdbusargument_p.h qdbusutil_p.h qdbusabstractinterface_p.h \
	   qdbuscontext_p.h qdbusthreaddebug_p.h qdbusintegrator_p.h \
	   qdbus_symbols_p.h

SOURCES += qdbusconnection.cpp  \
	qdbusconnectioninterface.cpp \
	qdbuserror.cpp		\
	qdbusintegrator.cpp	\
	qdbusmessage.cpp	\
	qdbusserver.cpp		\
	qdbusabstractinterface.cpp \
	qdbusinterface.cpp	\
	qdbusxmlparser.cpp	\
	qdbusutil.cpp		\
	qdbusintrospection.cpp	\
	qdbusabstractadaptor.cpp \
	qdbusthread.cpp \
	qdbusinternalfilters.cpp \
	qdbusmetaobject.cpp	\
	qdbusxmlgenerator.cpp	\
	qdbusmisc.cpp		\
	qdbusargument.cpp	\
	qdbusreply.cpp		\
	qdbusmetatype.cpp	\
	qdbusextratypes.cpp	\
	qdbusmarshaller.cpp	\
	qdbuscontext.cpp        \
	qdbus_symbols.cpp

