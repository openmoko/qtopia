!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_security_sandboxedexe


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_sxe)
DEPENDS*=\
    /src/server/core_server::persisted\

include(sandboxedexe.pro)
