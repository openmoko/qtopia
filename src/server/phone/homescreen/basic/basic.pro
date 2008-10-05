!qbuild{

qtopia_project(server_lib)
TARGET=qpe_phone_homescreen_basic

depends(server/core_server)
depends(server/phone/homescreen/abstract)
depends(server/infrastructure/messageboard)

enable_telephony:depends(server/phone/telephony/dialercontrol)

enable_cell:depends(server/phone/telephony/callpolicymanager/cell)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(basic.pri)
