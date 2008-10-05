!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_dialercontrol


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/pim/servercontactmodel::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\

include(dialercontrol.pro)
