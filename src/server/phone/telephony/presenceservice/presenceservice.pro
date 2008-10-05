!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_presenceservice

requires(enable_voip)

depends(server/core_server)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(server/infrastructure/messageboard)
depends(libraries/qtopiacollective)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

presenceservice.files=$$QTOPIA_DEPOT_PATH/services/Presence/qpe
presenceservice.path=/services/Presence
INSTALLS+=presenceservice

include(presenceservice.pri)
