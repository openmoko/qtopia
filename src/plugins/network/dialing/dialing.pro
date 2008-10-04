!qbuild{
qtopia_project(qtopia plugin)
TARGET=dialing
# Packaged by settings/network
CONFIG+=no_pkg
depends(libraries/qtopiacomm/network)
enable_cell:depends(libraries/qtopiaphone)
}

FORMS       =   dialingbase.ui \
                advancedbase.ui
                
HEADERS	    =   dialupplugin.h \
                dialup.h \
                config.h \
                dialing.h \
                advanced.h \
                dialstring.h 

                
SOURCES	    =   dialupplugin.cpp \
                dialup.cpp \
                config.cpp \
                dialing.cpp \
                advanced.cpp \
                dialstring.cpp

isEmpty(DIALING_NETWORK_CONFIGS) {
    DIALING_NETWORK_CONFIGS=dialup dialupIR
    enable_cell:DIALING_NETWORK_CONFIGS+=dialupGPRS
}
for(l,DIALING_NETWORK_CONFIGS) {
    conf.files+=$$device_overrides(/etc/network/$${l}.conf)
}
conf.path=/etc/network
INSTALLS+=conf

bin.files=\
    $$device_overrides(/src/plugins/network/dialing/ppp-network)\
    $$QTOPIA_DEPOT_PATH/bin/qtopia-pppd-internal
bin.path=/bin
bin.hint=script
INSTALLS+=bin

pics.files	= $$QTOPIA_DEPOT_PATH/pics/Network/dialup/*  
pics.path	= /pics/Network/dialup
pics.hint=pics
INSTALLS+=pics

icons.files     = $$QTOPIA_DEPOT_PATH/pics/Network/icons/dialup/*
icons.path      = /pics/Network/icons/dialup
icons.hint=pics
INSTALLS+=icons

