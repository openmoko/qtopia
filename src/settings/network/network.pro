qtopia_project(qtopia app)
TARGET=netsetup
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= networkui.h addnetwork.h 
                  
SOURCES		= networkui.cpp addnetwork.cpp main.cpp


depends(libraries/qtopiacomm)
enable_vpn {
    depends(libraries/qtopiacomm/vpn)
    SOURCES += vpnui.cpp
    HEADERS += vpnui.h
}

phone{
    !free_package|free_plus_binaries {
        HEADERS += wapui.h addwapui.h
        SOURCES += wapui.cpp addwapui.cpp
        FORMS    = gatewaybase.ui \
                   mmsbase.ui \
                   browserbase.ui
    }
}
    

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/netsetup.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=$$QTOPIA_DEPOT_PATH/help/html/netsetup*
phone{
    !free_package|free_plus_binaries {
        otaservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NetworkSetup
        otaservice.path=/etc/qds
        INSTALLS += otaservice
        depends(libraries/qtopiaphone)
    }
}

help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/netsetup/*
pics.path=/pics/netsetup
pics.hint=pics
service.files=$$QTOPIA_DEPOT_PATH/services/NetworkSetup/netsetup
service.path=/services/NetworkSetup
INSTALLS+=desktop help pics service

pkg.desc=Network configuration utility
pkg.multi=plugins/network/dialup plugins/network/lan
pkg.domain=window,net,netconfig
