!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_msgcontrol


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
enable_telephony:QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\

include(msgcontrol.pro)
