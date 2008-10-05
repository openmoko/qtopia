!qbuild{
qtopia_project(server_lib)
TARGET=qpe_processctrl_quickexe

requires(equals(LAUNCH_METHOD,quicklaunch))

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(quickexe.pri)
