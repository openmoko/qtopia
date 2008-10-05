!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_filetransferwindow


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/comm/filetransfer::persisted\
    /src/server/processctrl/taskmanagerentry::persisted\

include(filetransferwindow.pro)
