!qbuild{

requires(enable_cell)

qtopia_project(server_lib)
TARGET=qpe_phone_samples_e2

depends(server/core_server)
depends(server/phone/samples/e1)
depends(server/phone/browserscreen/abstract)
depends(server/ui/taskmanager)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(e2.pri)
