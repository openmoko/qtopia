!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_callpolicymanager_jabber

requires(enable_voip)

depends(server/core_server)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)
depends(libraries/qtopiacollective)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(jabber.pri)
