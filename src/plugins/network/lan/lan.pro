!qbuild{
qtopia_project(qtopia plugin)
TARGET=lan
depends(libraries/qtopiacomm/network)
# Packaged by settings/network
CONFIG+=no_pkg
}

FORMS       = wirelessbase.ui \
              wirelessencryptbase.ui \
              roamingbase.ui

HEADERS	    = lanplugin.h \
              lan.h \
              config.h \
              wirelessconfig.h \
              encryptionconfig.h \
              wirelessscan.h \
              roamingconfig.h \
              roamingmonitor.h \
              wnet.h\
              wlanregistrationprovider.h \
              wirelessipconfig.h
                
SOURCES	    = lanplugin.cpp \
              lan.cpp \
              config.cpp \
              wirelessconfig.cpp \
              encryptionconfig.cpp \
              wirelessscan.cpp \
              roamingconfig.cpp \
              roamingmonitor.cpp \
              wnet.cpp\
              wlanregistrationprovider.cpp \
              wirelessipconfig.cpp

isEmpty(LAN_NETWORK_CONFIGS):LAN_NETWORK_CONFIGS=lan lan-pcmcia wlan-pcmcia wlan
for(l,LAN_NETWORK_CONFIGS) {
    conf.files+=$$device_overrides(/etc/network/$${l}.conf)
}
conf.path=/etc/network
INSTALLS+=conf

bin.files=$$device_overrides(/src/plugins/network/lan/lan-network)
bin.path=/bin
bin.hint=script
INSTALLS+=bin

pics.files	= $$QTOPIA_DEPOT_PATH/pics/Network/lan/* 
pics.path	= /pics/Network/lan
pics.hint=pics
INSTALLS+=pics

icons.files     = $$QTOPIA_DEPOT_PATH/pics/Network/icons/lan/*
icons.path      = /pics/Network/icons/lan
icons.hint=pics
INSTALLS+=icons

