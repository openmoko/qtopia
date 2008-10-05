!qbuild_server_deps{
disable_project("does not compile. not maintained?")
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_launcherviews_hierarchdocumentview


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/ui/launcherviews/base::persisted\
    /src/server/ui/launcherviews/documentview::persisted\

include(hierarchdocumentview.pro)
