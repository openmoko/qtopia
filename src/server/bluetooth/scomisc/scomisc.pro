!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_scomisc

requires(enable_bluetooth)

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

# This is necessary for Handsfree / Headset to work
equals(QTOPIA_SOUND_SYSTEM,alsa) {
    !qbuild:depends(3rdparty/libraries/alsa)
    DEFINES+=HAVE_ALSA
}

include(scomisc.pri)
