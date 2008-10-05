!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_dialer_touch



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/dialer/abstract::persisted\
    /src/server/phone/media/dtmfaudio::persisted\
    /src/server/phone/themecontrol::persisted\
    /src/server/phone/telephony/dialfilter/abstract::persisted\
    /src/server/pim/savetocontacts::persisted\
    /src/server/pim/servercontactmodel::persisted\

include(touch.pro)
