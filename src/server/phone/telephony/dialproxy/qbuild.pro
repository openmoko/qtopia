!qbuild_server_deps{
#add dep when compiling
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_dialproxy


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\

include(dialproxy.pro)
