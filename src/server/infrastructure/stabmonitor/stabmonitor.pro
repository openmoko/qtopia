!qbuild{
qtopia_project(server_lib)
TARGET=qpe_infrastructure_stabmonitor

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(stabmonitor.pri)
