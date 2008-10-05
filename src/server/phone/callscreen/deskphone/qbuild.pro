!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_callscreen_deskphone




DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
MODULES*=homeui
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/callscreen/abstract::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\
    /src/server/pim/servercontactmodel::persisted\

include(deskphone.pro)
