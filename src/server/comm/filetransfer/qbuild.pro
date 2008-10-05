!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_comm_filetransfer


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)

}
DEPENDS*=\
    /src/server/core_server::persisted\

include(filetransfer.pro)
