!qbuild{
qtopia_project(server_lib)
TARGET=qpe_media_volumemanagement

depends(server/core_server)
depends(libraries/qtopiaaudio)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(volumemanagement.pri)
