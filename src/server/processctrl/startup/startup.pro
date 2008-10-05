!qbuild{
qtopia_project(server_lib)
TARGET=qpe_processctrl_startup

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(startup.pri)

# install the startup apps config file
bgapps_settings.files=$$device_overrides(/etc/default/Trolltech/BackgroundApplications.conf)
bgapps_settings.path=/etc/default/Trolltech
INSTALLS+=bgapps_settings
