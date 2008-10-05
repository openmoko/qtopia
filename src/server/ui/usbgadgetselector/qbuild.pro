!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_usbgadgetselector


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$path(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/comm/usbgadget::persisted\

include(usbgadgetselector.pro)
