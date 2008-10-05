!qbuild{
qtopia_project(server_lib)
TARGET=qpe_pim_buddysyncer

depends(server/core_server)
depends(libraries/qtopiapim)
depends(libraries/qtopiacollective)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(buddysyncer.pri)
