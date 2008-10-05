!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_processctrl_quickexe


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(equals(LAUNCH_METHOD,quicklaunch))
DEPENDS*=\
    /src/server/core_server::persisted\

include(quickexe.pro)
