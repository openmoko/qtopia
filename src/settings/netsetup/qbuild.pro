TEMPLATE=app
CONFIG+=qtopia singleexec quicklaunch
TARGET=netsetup

QTOPIA*=comm
enable_cell:QTOPIA*=phone

HEADERS		= networkui.h addnetwork.h 
SOURCES		= networkui.cpp addnetwork.cpp main.cpp

enable_vpn {
    SOURCES += vpnui.cpp
    HEADERS += vpnui.h
}

HEADERS += wapui.h addwapui.h
SOURCES += wapui.cpp addwapui.cpp
FORMS    = gatewaybase.ui \
           mmsbase.ui \
           browserbase.ui

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/netsetup.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=$$QTOPIA_DEPOT_PATH/help/html/netsetup*
help.hint=help
INSTALLS+=help
otaservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NetworkSetup
otaservice.path=/etc/qds
INSTALLS += otaservice
pics.files=$$QTOPIA_DEPOT_PATH/pics/netsetup/*
pics.path=/pics/netsetup
pics.hint=pics
INSTALLS+=pics
service.files=$$QTOPIA_DEPOT_PATH/services/NetworkSetup/netsetup
service.path=/services/NetworkSetup
INSTALLS+=service

pkg.desc=Network configuration utility
pkg.multi=\
    plugins/network/lan\
    plugins/network/bluetooth\
    plugins/network/bluetoothpan\
    plugins/network/dialing
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

