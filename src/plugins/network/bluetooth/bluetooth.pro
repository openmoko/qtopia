!qbuild{
qtopia_project(qtopia plugin)
TARGET=bluetooth
# Packaged by settings/network
CONFIG+=no_pkg
depends(libraries/qtopiacomm/network)
depends(libraries/qtopiacomm/bluetooth)
}

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

isEmpty(BLUETOOTH_NETWORK_CONFIGS):BLUETOOTH_NETWORK_CONFIGS=bluetoothDUN
for(l,BLUETOOTH_NETWORK_CONFIGS) {
    conf.files+=$$device_overrides(/etc/network/$${l}.conf)
}
conf.path=/etc/network
INSTALLS+=conf

bin.files=$$device_overrides(/src/plugins/network/bluetooth/btdun-network)
bin.path=/bin
bin.hint=script
INSTALLS+=bin

