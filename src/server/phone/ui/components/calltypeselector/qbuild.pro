!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_ui_components_calltypeselector


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\

include(calltypeselector.pro)
