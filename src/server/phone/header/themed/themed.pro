!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_header_themed

depends(server/core_server)
depends(server/phone/header/abstract)
depends(server/phone/themecontrol)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(themed.pri)
