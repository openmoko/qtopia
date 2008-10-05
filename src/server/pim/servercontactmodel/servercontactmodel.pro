!qbuild{
qtopia_project(server_lib)
TARGET=qpe_pim_servercontactmodel

depends(server/core_server)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(servercontactmodel.pri)
