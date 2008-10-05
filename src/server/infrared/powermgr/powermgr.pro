!qbuild{
qtopia_project(server_lib)
TARGET=qpe_infrared_powermgr

requires(enable_infrared)

depends(server/core_server)
depends(server/comm/session)
depends(libraries/qtopiacomm/ir)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(powermgr.pri)
