!qbuild{
requires(enable_telephony)

qtopia_project(server_lib)
TARGET=qpe_phone_media_dtmfaudio

depends(server/core_server)
depends(libraries/qtopiaphone)
depends(libraries/qtopiacomm)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dtmfaudio.pri)
