!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_powermgr

requires(enable_bluetooth)

depends(server/core_server)
depends(server/comm/session)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(powermgr.pri)
