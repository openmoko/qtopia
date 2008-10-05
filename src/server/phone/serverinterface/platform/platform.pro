!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_serverinterface_platform

depends(server/core_server)
depends(server/phone/browserscreen/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(platform.pri)
