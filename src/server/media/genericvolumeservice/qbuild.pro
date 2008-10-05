!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_media_genericvolumeservice


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=audio
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/media/volumemanagement::exists\

include(genericvolumeservice.pro)
