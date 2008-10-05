!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_media_audiohandler_dialtone

requires(enable_telephony)

depends(server/core_server)
depends(server/phone/media/audiohandler/abstract)
depends(server/phone/media/dtmfaudio)
depends(server/phone/telephony/callpolicymanager/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dialtone.pri)
