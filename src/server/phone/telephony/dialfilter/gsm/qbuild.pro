!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_dialfilter_gsm


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/dialfilter/abstract::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\

include(gsm.pro)
