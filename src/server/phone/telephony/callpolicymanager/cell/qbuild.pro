!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_callpolicymanager_cell


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\
    /src/server/pim/servercontactmodel::persisted\

include(cell.pro)
