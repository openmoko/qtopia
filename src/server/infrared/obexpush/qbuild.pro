!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_infrared_obexpush


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_infrared)
QTOPIA*=comm
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/comm/obex::persisted\
    /src/server/comm/filetransfer::persisted\
    /src/server/infrared/powermgr::exists\

include(obexpush.pro)
