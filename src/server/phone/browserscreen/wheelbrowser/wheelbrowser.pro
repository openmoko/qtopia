!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_browserscreen_wheelbrowser

depends(server/core_server)
depends(server/phone/browserscreen/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(wheelbrowser.pri)
