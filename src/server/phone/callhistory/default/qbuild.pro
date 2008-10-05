!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_callhistory_default



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/callhistory/abstract::persisted\
    /src/server/phone/ui/callcontactmodelview::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\

include(default.pro)
