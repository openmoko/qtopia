!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_browserscreen_deskphone


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/browserscreen/abstract::persisted\
    /src/server/phone/homescreen/abstract::persisted\
    /src/server/phone/themecontrol::persisted\
    /src/server/phone/ui/browserstack::persisted\
    /src/server/ui/launcherviews/base::persisted\

include(deskphone.pro)
