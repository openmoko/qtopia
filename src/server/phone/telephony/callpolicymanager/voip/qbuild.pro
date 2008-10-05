!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_callpolicymanager_voip


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_voip)
QTOPIA*=phone pim collective
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\

include(voip.pro)
