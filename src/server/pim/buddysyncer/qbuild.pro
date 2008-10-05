!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_pim_buddysyncer


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
QTOPIA*=pim collective
DEPENDS*=\
    /src/server/core_server::persisted\

include(buddysyncer.pro)
