!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_contextlabel_deskphone


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/contextlabel/abstract::persisted\
    /src/server/infrastructure/softmenubar::persisted\
    /src/server/phone/themecontrol::persisted\

include(deskphone.pro)
