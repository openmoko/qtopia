!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_media_audiohandler_call



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\

include(call.pro)
