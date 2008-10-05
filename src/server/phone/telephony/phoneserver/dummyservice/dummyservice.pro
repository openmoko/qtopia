!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_phoneserver_dummyservice

requires(enable_cell)

depends(server/core_server)
depends(libraries/qtopiaphone)
depends(server/phone/telephony/phoneserver/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dummyservice.pri)
