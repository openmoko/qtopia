CONFIG		+= qtopiaapp

HEADERS		= networksettings.h addnetworksetting.h
SOURCES		= networksettings.cpp addnetworksetting.cpp main.cpp
INTERFACES	= addnetworksettingbase.ui networksettingsbase.ui
TRANSLATABLES = $${HEADERS} $${SOURCES} $${INTERFACES}

TARGET		= netsetup


desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/netsetup.desktop
desktop.path=/apps/Settings

help.files=$${QTOPIA_DEPOT_PATH}/help/html/qppp-*
QTOPIA_PHONE {
    help.files+=$${QTOPIA_DEPOT_PATH}/help/html/netsetup*
    omaservice.files=$${QTOPIA_DEPOT_PATH}/services/Push/application/vnd.wap.connectivity-wbxml/netsetup
    omaservice.path=/services/Push/application/vnd.wap.connectivity-wbxml
    otaconfig.files=$${QTOPIA_DEPOT_PATH}/etc/network/OTAGPRS.conf
    otaconfig.path=/etc/network
    nokiaservice.files=$${QTOPIA_DEPOT_PATH}/services/Push/application/x-wap-prov.browser-settings/netsetup
    nokiaservice.path=/services/Push/application/x-wap-prov.browser-settings
    LIBS += -lqtopiawap
    INSTALLS += omaservice otaconfig nokiaservice
}
buildSingleExec:HEADERS += ../../plugins/network/proxiespage.h
buildSingleExec:SOURCES += ../../plugins/network/proxiespage.cpp
buildSingleExec:INTERFACES += ../../plugins/network/proxiespagebase_p.ui

TRANSLATABLES += ../../plugins/network/proxiespage.h \
                    ../../plugins/network/proxiespage.cpp \
                    ../../plugins/network/proxiespagebase_p.ui

help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/netsetup/*
pics.path=/pics/netsetup
INSTALLS	+= desktop help
PICS_INSTALLS+=pics

