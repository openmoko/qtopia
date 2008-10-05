!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_callpolicymanager_asterisk

requires(enable_voip)

depends(server/core_server)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(asterisk.pri)
