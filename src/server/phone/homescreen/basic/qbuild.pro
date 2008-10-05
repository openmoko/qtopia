!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_homescreen_basic

enable_telephony:DEPENDS*=\
        /src/server/phone/telephony/dialercontrol::persisted\

enable_cell:DEPENDS*=\
        /src/server/phone/telephony/callpolicymanager/cell::persisted\


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/homescreen/abstract::persisted\
    /src/server/infrastructure/messageboard::persisted\

include(basic.pro)
