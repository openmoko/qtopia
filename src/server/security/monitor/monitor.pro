!qbuild{
qtopia_project(server_lib)
TARGET=qpe_security_monitor

requires(enable_sxe)

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(monitor.pri)
