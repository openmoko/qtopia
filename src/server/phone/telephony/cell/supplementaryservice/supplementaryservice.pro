!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_cell_supplementaryservice

requires(enable_cell)

depends(server/core_server)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(supplementaryservice.pri)
