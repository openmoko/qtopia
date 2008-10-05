!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_callpolicymanager_cell

requires(enable_cell)

depends(server/core_server)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(server/pim/servercontactmodel)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(cell.pri)
