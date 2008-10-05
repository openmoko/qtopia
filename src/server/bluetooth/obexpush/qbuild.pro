!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_bluetooth_obexpush


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_bluetooth)
QTOPIA*=comm
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/comm/obex::persisted\
    /src/server/comm/filetransfer::persisted\
    /src/server/bluetooth/powermgr::exists\
    /src/server/bluetooth/servicemgr::exists\

include(obexpush.pro)
