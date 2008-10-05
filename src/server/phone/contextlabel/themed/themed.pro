!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_contextlabel_themed

depends(server/core_server)
depends(server/phone/contextlabel/base)
depends(server/phone/themecontrol)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(themed.pri)
