!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_bluetooth_servicemgr


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_bluetooth)
QTOPIA*=comm
DEPENDS*=\
    /src/server/core_server::persisted\

include(servicemgr.pro)
