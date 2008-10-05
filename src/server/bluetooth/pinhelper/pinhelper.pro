!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_pinhelper

requires(enable_bluetooth)

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(pinhelper.pri)
