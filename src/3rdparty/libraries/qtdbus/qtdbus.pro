qtopia_project(qtopia core lib)
license(FREEWARE)
TARGET          =   qtdbus
CONFIG+=no_tr

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QDBUS_MAKEDLL DBUS_API_SUBJECT_TO_CHANGE QDBUS_NO_SPECIALTYPES

depends(libraries/qt/xml)
depends(3rdparty/libraries/dbus)

INCLUDEPATH += $$DBUS_PREFIX/lib/dbus-1.0/include $$DBUS_PREFIX/include/dbus-1.0 $$QTE_DEPOT_PATH/tools/qdbus/src

VPATH += $$QTE_DEPOT_PATH/tools/qdbus/src

HEADERS =  qdbusabstractadaptor.h \
           qdbusabstractinterface.h \
           qdbusargument.h \
           qdbusconnection.h \
           qdbusconnectioninterface.h \
           qdbuserror.h \
           qdbusextratypes.h \
           qdbusinterface.h \
           qdbusmacros.h \
           qdbusmessage.h \
           qdbusmetatype.h \
           qdbusreply.h \
           qdbusserver.h

sdk_headers.files=$${HEADERS}
sdk_headers.path=/include/qtdbus
sdk_headers.hint=sdk headers
INSTALLS+=sdk_headers

CONFIG+=qtopia_visibility

PRIVATE_HEADERS += qdbusabstractadaptor_p.h \
                   qdbusabstractinterface_p.h \
                   qdbusargument_p.h \
                   qdbusconnection_p.h \
                   qdbusinterface_p.h \
                   qdbusintrospection_p.h \
                   qdbusmessage_p.h \
                   qdbusmetaobject_p.h \
                   qdbusmetatype_p.h \
                   qdbusutil_p.h \
                   qdbusxmlparser_p.h


HEADERS += $$PRIVATE_HEADERS

SOURCES += qdbusabstractadaptor.cpp \
           qdbusabstractinterface.cpp \
           qdbusargument.cpp \
           qdbusconnection.cpp \
           qdbusconnectioninterface.cpp \
           qdbusdemarshaller.cpp \
           qdbuserror.cpp \
           qdbusextratypes.cpp \
           qdbusintegrator.cpp \ 
           qdbusinterface.cpp \
           qdbusinternalfilters.cpp \
           qdbusintrospection.cpp \
           qdbusmarshaller.cpp \
           qdbusmessage.cpp \
           qdbusmetaobject.cpp \
           qdbusmetatype.cpp \
           qdbusmisc.cpp \
           qdbusreply.cpp \
           qdbusserver.cpp \
           qdbusthread.cpp \
           qdbusutil.cpp \
           qdbusxmlgenerator.cpp \
           qdbusxmlparser.cpp 

idep(LIBS+=-l$$TARGET)
idep(DEFINES+=QDBUS_NO_SPECIALTYPES)
qt_inc($$TARGET)
