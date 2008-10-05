!qbuild{
qtopia_project(server_lib)
TARGET=qpe_processctrl_terminationhandler

depends(server/core_server)
depends(server/processctrl/appmonitor)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(terminationhandler.pri)
