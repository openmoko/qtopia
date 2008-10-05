!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_comm_usbgadget


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=comm
DEPENDS*=\
    /src/server/core_server::persisted\

include(usbgadget.pro)
