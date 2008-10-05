!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_net_vpn


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_vpn)
QTOPIA*=comm
DEPENDS*=\
    /src/server/core_server::persisted\

include(vpn.pro)
