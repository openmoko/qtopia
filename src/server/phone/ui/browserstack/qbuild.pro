!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_ui_browserstack


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/processctrl/appmonitor::persisted\

include(browserstack.pro)
