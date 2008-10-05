!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_phoneserver_atmodemservice

requires(enable_cell)

depends(server/core_server)
depends(libraries/qtopiaphonemodem)
depends(server/phone/telephony/phoneserver/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(atmodemservice.pri)
