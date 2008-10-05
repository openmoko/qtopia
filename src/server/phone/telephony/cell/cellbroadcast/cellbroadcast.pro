!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_cellbroadcast

requires(enable_cell)

depends(server/core_server)
depends(server/infrastructure/messageboard)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(cellbroadcast.pri)
