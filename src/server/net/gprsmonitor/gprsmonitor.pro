!qbuild{
qtopia_project(server_lib)
TARGET=qpe_net_gprsmonitor

requires(enable_cell)

depends(server/core_server)
depends(libraries/qtopiacomm)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(gprsmonitor.pri)

