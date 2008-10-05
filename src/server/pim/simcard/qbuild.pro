!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_pim_simcard


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
QTOPIA*=phone pim
DEPENDS*=\
    /src/server/core_server::persisted\

include(simcard.pro)
