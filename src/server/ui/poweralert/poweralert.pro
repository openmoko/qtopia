!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_poweralert

depends(server/core_server)
depends(server/media/alertservice)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(poweralert.pri)
