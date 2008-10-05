!qbuild{
qtopia_project(server_lib)
TARGET=qpe_comm_session

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(session.pri)
