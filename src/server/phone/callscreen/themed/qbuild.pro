!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_callscreen_themed


enable_qtopiamedia {
    DEPENDS*=/src/server/phone/telephony/videoringtone::persisted\
}


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/callscreen/abstract::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\
    /src/server/phone/media/audiohandler/call::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\
    /src/server/phone/telephony/dialfilter/abstract::persisted\
    /src/server/phone/themecontrol::persisted\
    /src/server/pim/servercontactmodel::persisted\
    /src/server/processctrl/taskmanagerentry::persisted\

include(themed.pro)
