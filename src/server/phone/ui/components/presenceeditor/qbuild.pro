!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_ui_components_presenceeditor


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_voip)
QTOPIA*=phone collective pim
DEPENDS*=\
    /src/server/core_server::persisted\

include(presenceeditor.pro)
