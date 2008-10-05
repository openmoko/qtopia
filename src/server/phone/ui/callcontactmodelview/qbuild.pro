!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_ui_callcontactmodelview


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=pim phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/pim/savetocontacts::persisted\
    /src/3rdparty/libraries/inputmatch::persisted\

include(callcontactmodelview.pro)
