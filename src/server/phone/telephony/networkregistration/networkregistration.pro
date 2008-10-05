!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_networkregistration

requires(enable_telephony)

depends(server/core_server)
depends(libraries/qtopiaphone)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(server/infrastructure/messageboard)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(networkregistration.pri)
