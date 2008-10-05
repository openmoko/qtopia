!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_ui_components_simapp


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/infrastructure/messageboard::persisted\
    /src/server/processctrl/appmonitor::persisted\

include(simapp.pro)
