!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_dialfilter_abstract

requires(enable_telephony)

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(abstract.pri)

phone_settings.files=$$device_overrides(/etc/default/Trolltech/Phone.conf)
phone_settings.path=/etc/default/Trolltech
INSTALLS+=phone_settings
