!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_poweralert


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/media/alertservice::persisted\

include(poweralert.pro)
