!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_atemulator



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/tools/atinterface::exists
include(atemulator.pro)
