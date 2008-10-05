!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_taskmanager


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/ui/abstractinterfaces/taskmanager::persisted\
    /src/server/ui/launcherviews/base::persisted\

include(taskmanager.pro)
