!qbuild{
qtopia_project(server_lib)
TARGET=qpe_media_genericvolumeservice

requires(contains(PROJECTS,server/media/volumemanagement))

depends(server/core_server)
depends(libraries/qtopiaaudio)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(genericvolumeservice.pri)

sound_settings.files=$$device_overrides(/etc/default/Trolltech/Sound.conf)
!isEmpty(settings.files) {
    sound_settings.path=/etc/default/Trolltech
    INSTALLS+=sound_settings
}
