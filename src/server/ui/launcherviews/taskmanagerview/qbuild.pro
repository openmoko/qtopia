!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_launcherviews_taskmanagerview


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/ui/launcherviews/base::persisted\
    /src/server/processctrl/appmonitor::persisted\
    /src/server/processctrl/taskmanagerentry::persisted\

include(taskmanagerview.pro)
