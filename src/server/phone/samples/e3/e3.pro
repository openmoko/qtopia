!qbuild{

requires(enable_cell)

qtopia_project(server_lib)
TARGET=qpe_phone_samples_e3

depends(server/core_server)
depends(server/phone/browserscreen/abstract)
depends(server/phone/callhistory/abstract)
depends(server/phone/callscreen/abstract)
depends(server/phone/contextlabel/abstract)
depends(server/phone/dialer/abstract)
depends(server/phone/header/abstract)
depends(server/phone/telephony/callpolicymanager/abstract)
depends(server/phone/telephony/dialproxy)
depends(server/phone/themecontrol)
depends(server/phone/ui/browserstack)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(e3.pri)

samples_settings.files=\
    $$QTOPIA_DEPOT_PATH/etc/default/Trolltech/E3.conf
samples_settings.path=/etc/default/Trolltech
INSTALLS+=samples_settings
