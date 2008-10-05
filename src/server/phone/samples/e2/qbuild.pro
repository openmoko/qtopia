!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_samples_e2



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_cell)
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/samples/e1::persisted\
    /src/server/phone/browserscreen/abstract::persisted\
    /src/server/ui/taskmanager::persisted\

include(e2.pro)
