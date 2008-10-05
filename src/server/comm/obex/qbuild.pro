!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_comm_obex


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=comm pim
DEPENDS*=\
    /src/server/core_server::persisted\

include(obex.pro)
