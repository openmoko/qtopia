!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_dialercontrol

requires(enable_telephony)

depends(server/core_server)
depends(libraries/qtopiaphone)
depends(server/pim/servercontactmodel)
depends(server/phone/telephony/callpolicymanager/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dialercontrol.pri)
