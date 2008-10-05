TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiacomm
VERSION=4.0.0

MODULE_NAME=qtopiacomm
QT*=network
CONFIG+=i18n use_default_i18n_rules

include(qtopiacomm.pri)

contains(PROJECTS,3rdparty/libraries/openobex) {
    MODULES*=openobex
    include(obex/obex.pri)
}
enable_bluetooth:include(bluetooth/bluetooth.pri)
enable_infrared:include(ir/ir.pri)
include(network/network.pri)
include(serial/serial.pri)
enable_vpn:include(vpn/vpn.pri)
include(usb/usb.pri)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

