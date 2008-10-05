!qbuild{
qtopia_project(server_lib)
TARGET=qpe_pim_simcard

requires(enable_cell)

depends(server/core_server)
depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(simcard.pri)
