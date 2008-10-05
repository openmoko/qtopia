!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_cell_simcontrolevent


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\

include(simcontrolevent.pro)
