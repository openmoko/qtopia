!qbuild_server_deps{

TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_media_audiohandler_abstract



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=audio
DEPENDS*=\
    /src/server/core_server::persisted\

include(abstract.pro)
