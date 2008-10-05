!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_profileprovider


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=audio
DEPENDS*=\
    /src/server/core_server::persisted\

include(profileprovider.pro)
