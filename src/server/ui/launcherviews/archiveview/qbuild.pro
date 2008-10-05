!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_launcherviews_archiveview


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(drmagent)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/ui/launcherviews/base::persisted\

include(archiveview.pro)
