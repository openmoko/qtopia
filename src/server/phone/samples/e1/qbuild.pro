!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_samples_e1



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\
    /src/server/phone/telephony/dialproxy::persisted\
    /src/server/phone/themecontrol::persisted\
    /src/server/phone/ui/browserstack::persisted\
    /src/server/ui/launcherviews/base::persisted\

include(e1.pro)
