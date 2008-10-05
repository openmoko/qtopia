!qbuild{
qtopia_project(server_lib)
TARGET=qpe_comm_obex
    
depends(server/core_server)
depends(libraries/qtopiacomm/obex)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(obex.pri)
