qtopia_project(qtopia plugin)
TARGET=bluetooth

# Packaged by settings/network
CONFIG+=no_pkg

FORMS       = advancedbtbase.ui dialingbtbase.ui
                
HEADERS	    =   bluetoothplugin.h \
                bluetoothimpl.h \
                config.h \
                btdialupdevice.h \
                configui.h
                
SOURCES	    =   bluetoothplugin.cpp \
                bluetoothimpl.cpp \
                config.cpp \
                btdialupdevice.cpp \
                configui.cpp

depends(libraries/qtopiacomm/network)
depends(libraries/qtopiacomm/bluetooth)
                
conf.files	= $$QTOPIA_DEPOT_PATH/etc/network/bluetoothDUN.conf

conf.path	= /etc/network

bin.files   = $$QTOPIA_DEPOT_PATH/src/plugins/network/bluetooth/btdun-network
bin.path    = /bin
bin.hint=script

INSTALLS += conf bin

pics.files	= $$QTOPIA_DEPOT_PATH/pics/Network/bluetooth/*  
pics.path	= /pics/Network/bluetooth
pics.hint=pics
icons.files     = $$QTOPIA_DEPOT_PATH/pics/Network/icons/*
icons.path      = /pics/Network/icons
icons.hint=pics
INSTALLS	+= icons pics

