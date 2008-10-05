!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_media_volumemanagement


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=audio
DEPENDS*=\
    /src/server/core_server::persisted\

include(volumemanagement.pro)
