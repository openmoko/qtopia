!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_ui_browserstack

depends(server/core_server)
depends(server/processctrl/appmonitor)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(browserstack.pri)
