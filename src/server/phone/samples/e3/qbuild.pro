!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_samples_e3



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/browserscreen/abstract::persisted\
    /src/server/phone/callhistory/abstract::persisted\
    /src/server/phone/callscreen/abstract::persisted\
    /src/server/phone/contextlabel/abstract::persisted\
    /src/server/phone/dialer/abstract::persisted\
    /src/server/phone/header/abstract::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\
    /src/server/phone/telephony/dialproxy::persisted\
    /src/server/phone/themecontrol::persisted\
    /src/server/phone/ui/browserstack::persisted\
    /src/server/ui/launcherviews/base::persisted\

include(e3.pro)
